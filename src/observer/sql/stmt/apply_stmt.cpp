#include "sql/stmt/apply_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"
#include "sql/operator/apply_logical_operator.h"
#include "sql/optimizer/logical_plan_generator.h"

void ApplyStmt::set_sub_query_stmt(std::unique_ptr<SelectStmt> sub_query)
{
    sub_query_ = std::move(sub_query);
}

void ApplyStmt::add_correlate_expr(CorrelateExpr *expr)
{
    correlate_exprs_.push_back(expr);
}

void ApplyStmt::add_correlate_exprs(const std::vector<CorrelateExpr*> &exprs)
{
    correlate_exprs_.insert(correlate_exprs_.end(), exprs.begin(), exprs.end());
}

RC ScalarSubqueryApplyStmt::create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper)
{
    std::unique_ptr<LogicalOperator> subquery_logic_plan; 
    RC rc = LogicalPlanGenerator().create(sub_query_.get(), subquery_logic_plan);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to create subquery logic plan. rc=%s", strrc(rc));
        return rc;
    }
    logic_oper = std::make_unique<ScalarSubqueryLogicalOperator>(field_identifier_in_expr_, 
        std::move(subquery_logic_plan), std::move(correlate_exprs_));
    return rc;
}

RC ExistentialSubqueryApplyStmt::create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper)
{
    std::unique_ptr<LogicalOperator> subquery_logic_plan; 
    RC rc = LogicalPlanGenerator().create(sub_query_.get(), subquery_logic_plan);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to create subquery logic plan. rc=%s", strrc(rc));
        return rc;
    }
    logic_oper = std::make_unique<ExistentialSubqueryLogicalOperator>(field_identifier_in_expr_, 
        std::move(subquery_logic_plan), std::move(correlate_exprs_), op_);
    return rc;
}

RC QuantifiedCompSubqueryApplyStmt::create_logic_plan(std::unique_ptr<LogicalOperator> &logic_oper)
{
    std::unique_ptr<LogicalOperator> subquery_logic_plan; 
    RC rc = LogicalPlanGenerator().create(sub_query_.get(), subquery_logic_plan);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to create subquery logic plan. rc=%s", strrc(rc));
        return rc;
    }
    logic_oper = std::make_unique<QuantifiedCompSubqueryLogicalOperator>(field_identifier_in_expr_, 
        std::move(subquery_logic_plan), std::move(correlate_exprs_), std::move(expr_), op_);
    return rc;
}

