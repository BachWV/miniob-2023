#include "sql/stmt/apply_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"

void ApplyStmt::set_sub_query_stmt(std::unique_ptr<SelectStmt> sub_query)
{
    sub_query_ = std::move(sub_query);
}

void ApplyStmt::add_correlate_expr(CorrelateExpr *expr)
{
    correlate_exprs_.push_back(expr);
}