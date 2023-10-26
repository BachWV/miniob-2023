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

/* 为子查询生成唯一的虚拟列名 */
class SubqueryIdentifierGenerator
{
public:
  FieldIdentifier generate_identifier() {
    // '-'号保证不会和其它列名冲突
    return FieldIdentifier("subquery-" + std::to_string(next_identifier_++));
  }

private:
  size_t next_identifier_ = 0;
};

/* 描述HAVING表达式中的聚集函数 */
class AggExprInfo
{
public:
  AggExprInfo(AggregateOp op, const FieldIdentifier &agg_field, const std::string &full_name, bool is_in_select_column)
    : op_(op), agg_field_(agg_field), full_name_(full_name), is_in_select_column_(is_in_select_column) {}
  
  AggregateOp agg_op() const { return op_; }
  const FieldIdentifier &agg_field() const { return agg_field_; }
  const std::string &full_name() const { return full_name_; }
  bool is_in_select_column() const { return is_in_select_column_; }

private:
  AggregateOp op_;
  FieldIdentifier agg_field_;  // 如果是'*'，则field_name为'*'。否则这里同时包含了表名和列名
  std::string full_name_;  // 整个聚集函数的字符串表示
  bool is_in_select_column_;  // 这个聚集函数是否在select的列中出现过了
};

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
  
  /* 解析聚集函数等虚拟字段的名字 */
  RC resolve_other_column_name(const std::string &col_name) const;

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

  SubqueryIdentifierGenerator& get_subquery_identifier_generator() { return subquery_identifier_generator_; }

private:
  std::vector<StmtResolveContext*> contexts_;
  SubqueryIdentifierGenerator subquery_identifier_generator_;
};

class ExprResolveResult
{
public:
  void set_result_expr_tree(std::unique_ptr<Expression> result_expr_tree);

  // 在被reslove处理的表达式树中，有一个标识符被解析为相关表达式expr，引用了第level层的查询，那么该标识符被转化为CorrelateExpr
  void add_correlate_expr(size_t level, CorrelateExpr *expr);
  void add_correlate_exprs(std::unordered_map<size_t, std::vector<CorrelateExpr*>> &exprs);
  void add_subquery(std::unique_ptr<ApplyStmt> subquery);

  // 调用者在调用后，不应再使用subquerys，因为内部会进行移动
  void add_subquerys(std::vector<std::unique_ptr<ApplyStmt>> &subquerys);

  void add_agg_expr_info(AggExprInfo &&info) { agg_expr_infos_.emplace_back(std::move(info)); }

  std::vector<std::unique_ptr<ApplyStmt>>& get_subquerys_in_expr() { return subquerys_; }
  std::unordered_map<size_t, std::vector<CorrelateExpr*>> &get_correlate_exprs();
  std::vector<AggExprInfo>& get_agg_expr_infos() { return agg_expr_infos_; }

  // 调用者获取resolve后的表达式树的所有权
  std::unique_ptr<Expression> owns_result_expr_tree();

private:
  std::unique_ptr<Expression> result_expr_tree_;

  /* 
   * 若两个Subquery在执行时有先后顺序，A必须先于B，则A的下标小于B的下标
   * 这种情况出现与集合比较子查询：expr in (sub_query)，且expr中也有子查询的情况
   * epxr中的子查询要先于sub_query完成
   */
  std::vector<std::unique_ptr<ApplyStmt>> subquerys_;

  /*
   * 表达式中引用上层作用域的，也包含子查询表达式中引用上层作用域的
   * 子查询表达式中引用本层作用域的相关表达式，在对应的resolve方法中，在ApplyStmt构造时加入到ApplyStmt中
   */
  std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_exprs_;

  /*
   * Having表达式中使用，包含聚集函数的信息
   */
  std::vector<AggExprInfo> agg_expr_infos_;
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
  ScalarSubquery,
  ExistentialSubquery,
  QuantifiedCompSubquery,
  AggIdentifier,
  Like,
  QuantifiedCompExprSet,
};

class ExprSqlNode
{
public:
  explicit ExprSqlNode(const std::string &expr_name, ExprSqlNodeType type) : expr_name_(expr_name), type_(type) {}
  virtual ~ExprSqlNode() = default;

  // name
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

/* 表达式是一个标量子查询 */
class ScalarSubqueryExprSqlNode: public ExprSqlNode
{
public:
  ScalarSubqueryExprSqlNode(const std::string &expr_name, ParsedSqlNode *sql)
    : ExprSqlNode(expr_name, ExprSqlNodeType::ScalarSubquery), sql_(sql) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ParsedSqlNode> sql_;
};

/* 表达式是一个存在性检测子查询 */
class ExistentialSubqueryExprSqlNode: public ExprSqlNode
{
public:
  ExistentialSubqueryExprSqlNode(const std::string &expr_name, ParsedSqlNode *sql, ExistOp op)
    : ExprSqlNode(expr_name, ExprSqlNodeType::ExistentialSubquery), sql_(sql), op_(op) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ParsedSqlNode> sql_;
  ExistOp op_;
};

/* 表达式是一个集合比较子查询 expr IN/NOT IN subquery */
class QuantifiedCompSubqueryExprSqlNode: public ExprSqlNode
{
public:
  QuantifiedCompSubqueryExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> child, ParsedSqlNode *sql, QuantifiedComp op)
    : ExprSqlNode(expr_name, ExprSqlNodeType::QuantifiedCompSubquery), child_(std::move(child)), sql_(sql), op_(op) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  std::unique_ptr<ExprSqlNode> child_;
  std::unique_ptr<ParsedSqlNode> sql_;
  QuantifiedComp op_;
};

/* 表示一个聚集函数名 */
class AggIdentifierExprSqlNode: public ExprSqlNode
{
public:
  AggIdentifierExprSqlNode(const AggregateFuncSqlNode &agg_func)
    : ExprSqlNode(agg_func.name, ExprSqlNodeType::AggIdentifier), op_(agg_func.agg_op),
    agg_field_(agg_func.attr.relation_name, agg_func.attr.attribute_name) { }

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  AggregateOp op_;
  FieldIdentifier agg_field_;
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

class FunctionExprSqlNode: public ExprSqlNode{
public:

private:
  
};

/* expr in/not in (expr, expr, ...) */
class QuantifiedCmpExprSetExprSqlNode: public ExprSqlNode
{
public:
  QuantifiedCmpExprSetExprSqlNode(const std::string &expr_name, std::unique_ptr<ExprSqlNode> child, QuantifiedComp op, ExprSqlSet *expr_set)
    : ExprSqlNode(expr_name, ExprSqlNodeType::QuantifiedCompExprSet), child_(std::move(child)), op_(op), expr_set_(expr_set) {}

  RC resolve(ExprResolveContext *ctx, ExprResolveResult *result) const override;

private:
  // child_ op_ (expr_set_)
  std::unique_ptr<ExprSqlNode> child_;
  QuantifiedComp op_;
  std::unique_ptr<ExprSqlSet> expr_set_;
};