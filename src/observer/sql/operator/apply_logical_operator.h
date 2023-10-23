#pragma once

#include <memory>

#include "sql/operator/logical_operator.h"
#include "storage/field/field.h"
#include "sql/parser/parse_defs.h"

class ApplyLogicalOperator: public LogicalOperator
{
public:
    ApplyLogicalOperator(const FieldIdentifier &identifier, std::unique_ptr<LogicalOperator> sub_query_logic_plan, 
        std::vector<CorrelateExpr*> &&correlate_exprs)
        : field_identifier_in_expr_(identifier), sub_query_logic_plan_(std::move(sub_query_logic_plan)), 
          correlate_exprs_(std::move(correlate_exprs)) {}
    
    const FieldIdentifier &get_field_identifier() { return field_identifier_in_expr_; }
    std::unique_ptr<LogicalOperator> fetch_subquery_logic_plan() { return std::move(sub_query_logic_plan_); }
    std::vector<CorrelateExpr*>& fetch_correlate_exprs() { return correlate_exprs_; }

protected:
    FieldIdentifier field_identifier_in_expr_;  // 子查询在表达式中的名字
    std::unique_ptr<LogicalOperator> sub_query_logic_plan_;  // 子查询的logic plan
    std::vector<CorrelateExpr*> correlate_exprs_;  // 子查询中引用ApplyLogicalOperator所在层的相关表达式
};

class ScalarSubqueryLogicalOperator : public ApplyLogicalOperator
{
public:
    ScalarSubqueryLogicalOperator(const FieldIdentifier &identifier, std::unique_ptr<LogicalOperator> sub_query_logic_plan, 
        std::vector<CorrelateExpr*> &&correlate_exprs)
        : ApplyLogicalOperator(identifier, std::move(sub_query_logic_plan), std::move(correlate_exprs)) {}

    LogicalOperatorType type() const override { return LogicalOperatorType::SCALAR_SUBQUERY; }
};

class ExistentialSubqueryLogicalOperator : public ApplyLogicalOperator
{
public:
    ExistentialSubqueryLogicalOperator(const FieldIdentifier &identifier, std::unique_ptr<LogicalOperator> sub_query_logic_plan, 
        std::vector<CorrelateExpr*> &&correlate_exprs, ExistOp op)
        : ApplyLogicalOperator(identifier, std::move(sub_query_logic_plan), std::move(correlate_exprs)), op_(op) {}

    LogicalOperatorType type() const override { return LogicalOperatorType::EXISTENTIAL_SUBQUERY; }

    ExistOp get_exist_op() { return op_; }

protected:
    ExistOp op_;
};

class QuantifiedCompSubqueryLogicalOperator : public ApplyLogicalOperator
{
public:
    QuantifiedCompSubqueryLogicalOperator(const FieldIdentifier &identifier, std::unique_ptr<LogicalOperator> sub_query_logic_plan, 
        std::vector<CorrelateExpr*> &&correlate_exprs, std::unique_ptr<Expression> expr, QuantifiedComp op)
    : ApplyLogicalOperator(identifier, std::move(sub_query_logic_plan), std::move(correlate_exprs)), 
    expr_(std::move(expr)), op_(op) {}

    LogicalOperatorType type() const override { return LogicalOperatorType::QUANTIFIEDCOMP_SUBQUERY; }

    std::unique_ptr<Expression> owns_expr() { return std::move(expr_); }
    QuantifiedComp get_quantified_comp_op() { return op_; }

protected:
    QuantifiedComp op_;
    std::unique_ptr<Expression> expr_;  // expr in subquery的expr
};