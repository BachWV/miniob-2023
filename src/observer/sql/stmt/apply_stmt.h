#pragma once

#include <string>
#include <vector>
#include <memory>

#include "common/rc.h"
#include "storage/field/field.h"
#include "sql/parser/parse_defs.h"

class CorrelateExpr;
class SelectStmt;
class LogicalOperator;
class Expression;

/* 
 * 包含构造Apply算子的全部信息
 * Apply算子，对应一个返回标量值的，包含子查询的表达式
 * 在resolver阶段，包含子查询的表达式用一个field代替，表达式转化为ApplyStmt，用于在logic_plan阶段构造apply算子 
 */
class ApplyStmt
{
public:
    ApplyStmt() = default;
    virtual ~ApplyStmt() = default;

    void set_field_identifier_in_expr(const FieldIdentifier &name) { field_identifier_in_expr_ = name; }
    void set_sub_query_stmt(std::unique_ptr<SelectStmt> sub_query);
    void add_correlate_expr(CorrelateExpr *expr);
    void add_correlate_exprs(const std::vector<CorrelateExpr*> &exprs);

    virtual RC create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper) = 0;

protected:
    FieldIdentifier field_identifier_in_expr_;  // 替代此表达式的field名字
    std::unique_ptr<SelectStmt> sub_query_;  // 子查询的SelectStmt
    std::vector<CorrelateExpr*> correlate_exprs_;  // 子查询中引用ApplyStmt所属查询的表达式
};

/* 标量子查询的ApplyStmt */
class ScalarSubqueryApplyStmt : public ApplyStmt
{
public:
    RC create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper) override;
};

/* 存在性检测子查询 */
class ExistentialSubqueryApplyStmt: public ApplyStmt
{
public:
    ExistentialSubqueryApplyStmt(ExistOp op) : op_(op) {}

    RC create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper) override;

private:
    ExistOp op_;
};

/* 集合比较子查询 */
class QuantifiedCompSubqueryApplyStmt: public ApplyStmt
{
public:
    QuantifiedCompSubqueryApplyStmt(std::unique_ptr<Expression> expr, QuantifiedComp op) : expr_(std::move(expr)), op_(op) {}

    RC create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper) override;

private:
    QuantifiedComp op_;
    std::unique_ptr<Expression> expr_;
};