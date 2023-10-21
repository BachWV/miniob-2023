#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <optional>

#include "common/rc.h"
#include "sql/parser/value.h"
#include "sql/stmt/select_stmt.h"
#include "storage/field/field.h"
#include "sql/parser/parse_defs.h"

class Expression;
class Table;
class CorrelateExpr;
class ExprSqlNode;
class ApplyStmt;

class StmtResolveContext
{
public:
  void add_table_to_namespace(const std::string &table_name, Table *table) {
    table_namespace_.emplace(table_name, table);
  }

  void add_other_column_name(const std::string &field_name) {
    other_column_names_.emplace(field_name);
  }

  /* 解析引用表中某一列的名字。解析成功（找不到也算成功，都返回RC::SUCCESS）。解析中出现错误（如出现歧义），返回其错误码 */
  RC resolve_table_field_identifier(const std::string &table_name, const std::string &field_name,
    std::optional<FieldIdentifier> &result) const;

private:
  /*
   * 包含了表名，或者在该stmt中该表的别名，到Table对象的映射
   * 此处仅简单的将名字映射到Table对象，将一些复杂度交给调用者Stmt（目前就是SelectStmt）：
   * 如果一个表有别名，那么这里只允许添加别名，不允许表名(MySQL规定的，如果错了再改)
   * 
   * 目前不考虑列别名
   */
  std::unordered_map<std::string, Table *> table_namespace_;

  /*
   * 表达式还可能引用虚拟字段名，如HAVING子句中：
   * HAVING AVG(id) > 1
   * 其中的AVG(id)可以是一个表达式，但它只能作为一个字段的名字表达式，用于找到tuple中该聚集函数的结果值，而不是一个需要处理的聚集函数
   * 如果用户对列使用了别名，那么该别名可以添加到这里，对列别名的解析在ProjTuple中进行（题目中不要求列解析，如果测例出现了就这么做）
   */
  std::unordered_set<std::string> other_column_names_;
};

/*
 * 用于解析Stmt中表达式的上下文
 * 每一层Stmt，在解析前，负责将自己的上下文构造好，并push进此对象
 * 解析后，Stmt负责将自己的上下文pop出去
 */
class ExprResolveContext
{
public:
  const StmtResolveContext& get_ith_level_stmt_ctx(size_t level) const { return *contexts_[level]; }
  size_t current_level() const { return contexts_.size() - 1; }

  void push_stmt_ctx(StmtResolveContext *ctx) { contexts_.push_back(ctx); }
  void pop_stmt_ctx() { contexts_.pop_back(); }

private:
  std::vector<StmtResolveContext*> contexts_;
};

class SubQueryInfo
{
public:
  // 调用者得到ApplyStmt的所有权
  std::unique_ptr<ApplyStmt> owns_apply_stmt();

  std::unordered_map<size_t, std::vector<CorrelateExpr*>> &correlate_exprs() { return correlate_exprs_; }

private:
  /*
   * 该子查询表达式的ApplyStmt表达
   * 在resolve中返回时，这个ApplyStmt中correlate_exprs_是空的，它无法在resolve时构造，因为一个子查询可以引用所有外层查询的名字，
   * 而ApplyStmt的correlate_exprs_只负责子查询中，引用当前层(也就是Apply算子所在的层)的相关表达式的求值，
   * 那么引用更上层的那些相关表达式，就要在SelectStmt构造时再继续往外传，所以下面返回了一个map，表示子查询引用第level层的相关表达式
   * SelectStmt构造时，负责将子查询中与自己这层相关的那些表达式设置到ApplyStmt中
   * 
   * 不同类型的子查询表达式(in/exists/any/all)的ApplyStmt是不同的，所以ApplyStmt需要在resolve时构造具体类，并在这里返回
   */
  std::unique_ptr<ApplyStmt> apply_stmt_;

  /*
    * map<level, correlate_exprs>
    * level: 表示sub_query_引用了第level层Stmt的数据
    *    这里不说是SelectStmt，因为其它类型的Sql中可能也包含子查询，子查询现在是一个合法表达式。当然目前只有SelectStmt
    *    Stmt的level从0开始，0表示最外层的Stmt，每嵌套一个子查询就多一层，同一层可以有多个子查询，如：
    *    select * from t1 where (select max(a) from t2 where t2.b = t1.b) > (select min(a) from t3 where t3.b = t1.b)中，
    *    外层查询的level是0，两个子查询的level都是1.可以想象，如果有多个子查询，那么level就是一个树状结构，每个节点都是一个Stmt
    *    这里的level仅用于处理一条路径上的Stmt，所以根据处理的状态，level可以唯一确定一个查询
    * 
    * std::vector<CorrelateExpr*>: 表示子查询中引用了第level层Stmt数据的表达式集合
    */
  std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_exprs_;
};

class ExprResolveResult
{
public:
  void set_result_expr_tree(std::unique_ptr<Expression> result_expr_tree);

  // 在被reslove处理的表达式树中，有一个标识符被解析为相关表达式expr，引用了第level层的查询，那么该标识符被转化为CorrelateExpr
  void add_correlate_expr(size_t level, CorrelateExpr *expr);
  void add_correlate_exprs(std::unordered_map<size_t, std::vector<CorrelateExpr*>> &exprs);

  // 调用者在调用后，不应再使用subquerys，因为内部会进行移动
  void add_subquerys(std::vector<SubQueryInfo> &subquerys);

  std::vector<SubQueryInfo>& get_subquerys_in_expr() { return subquerys_; }
  std::unordered_map<size_t, std::vector<CorrelateExpr*>> &get_correlate_exprs();

  // 调用者获取resolve后的表达式树的所有权
  std::unique_ptr<Expression> owns_result_expr_tree();

private:
  std::unique_ptr<Expression> result_expr_tree_;
  std::vector<SubQueryInfo> subquerys_;
  std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_exprs_;  // 表达式中引用上层作用域的。同上，map<level, exprs>
};

/* **************************** */
enum class ExprSqlNodeType
{
  Value,
  Identifier,
  NegativeArith,
  BinaryArith,
  Compare,
  PredNull,
  SubQuery,
  Like,
};

class ExprSqlNode
{
public:
  explicit ExprSqlNode(const std::string &expr_name, ExprSqlNodeType type) : expr_name_(expr_name), type_(type) {}
  virtual ~ExprSqlNode() = default;

  const std::string &expr_name() const { return expr_name_; }

  ExprSqlNodeType get_type() const { return type_; }

  virtual RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const = 0;

protected:
  std::string expr_name_;
  ExprSqlNodeType type_;
};

class ValueExprSqlNode : public ExprSqlNode
{
public:
  ValueExprSqlNode(const std::string &expr_name, const Value &value) : ExprSqlNode(expr_name, ExprSqlNodeType::Value), value_(value) {}
  ValueExprSqlNode(const std::string &expr_name, Value &&value) : ExprSqlNode(expr_name, ExprSqlNodeType::Value), value_(std::move(value)) {}
  virtual ~ValueExprSqlNode() = default;

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

protected:
  Value value_;
};

class IdentifierExprSqlNode : public ExprSqlNode
{
public:
  IdentifierExprSqlNode(const std::string &expr_name, const std::string &table_name, const std::string &field_name)
    : ExprSqlNode(expr_name, ExprSqlNodeType::Identifier), table_name(table_name), field_name(field_name) {}
  virtual ~IdentifierExprSqlNode() = default;

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

protected:
  std::string table_name;
  std::string field_name;
};

class NegativeArithExprSqlNode: public ExprSqlNode
{
public:
  NegativeArithExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> sub_expr)
    : ExprSqlNode(expr_name, ExprSqlNodeType::NegativeArith), sub_expr_(std::move(sub_expr)) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ExprSqlNode> sub_expr_;
};

class BinaryArithExprSqlNode: public ExprSqlNode
{
public:
  enum class BinaryArithType {
    Add,
    Sub,
    Mul,
    Div,
  };

  BinaryArithExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> left, std::unique_ptr<ExprSqlNode> right, 
    BinaryArithType type): ExprSqlNode(expr_name, ExprSqlNodeType::BinaryArith), left_(std::move(left)), right_(std::move(right)) {
      type_ = type;
    }

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ExprSqlNode> left_, right_;
  BinaryArithType type_;
};

class CompareExprSqlNode: public ExprSqlNode
{
public:
  CompareExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> left, std::unique_ptr<ExprSqlNode> right, CompOp op)
    : ExprSqlNode(expr_name, ExprSqlNodeType::Compare), left_(std::move(left)), right_(std::move(right)), op_(op) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ExprSqlNode> left_, right_;
  CompOp op_;
};

class PredNullExprSqlNode: public ExprSqlNode
{
public:
  PredNullExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> sub_expr, PredNullOp op)
    : ExprSqlNode(expr_name, ExprSqlNodeType::PredNull), sub_expr_(std::move(sub_expr)), op_(op) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ExprSqlNode> sub_expr_;
  PredNullOp op_;
};

class SubQueryExprSqlNode: public ExprSqlNode
{
public:
  SubQueryExprSqlNode(const std::string &expr_name, ParsedSqlNode *sql)
    : ExprSqlNode(expr_name, ExprSqlNodeType::SubQuery), sql_(sql) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ParsedSqlNode> sql_;
};

class LikeExprSqlNode: public ExprSqlNode
{
public:
  LikeExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> left, std::string pattern, bool is_not_like)
    : ExprSqlNode(expr_name, ExprSqlNodeType::Like), left_(std::move(left)), pattern_(std::move(pattern)), is_not_like_(is_not_like) {}
  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ExprSqlNode> left_;
  std::string pattern_;
  bool is_not_like_;
};