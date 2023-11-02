#include "sql/view/view.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/apply_stmt.h"
#include "sql/expr/expression.h"
#include "storage/db/db.h"

ResolvedView::ResolvedView(View *view, std::unique_ptr<SelectStmt> select)
    : select_(std::move(select)) 
{
    view_ = view;    
}

RC View::resolve_select(Db *db, ExprResolveContext *glob_ctx, std::unique_ptr<SelectStmt> &result) const
{
    Stmt *select_stmt;
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_in_select;
    RC rc = SelectStmt::create(db, glob_ctx, *select_def_, select_stmt, &correlate_in_select);
    result.reset(dynamic_cast<SelectStmt*>(select_stmt));
    if (OB_FAIL(rc)) {
        LOG_ERROR("failed to create select stmt in view def. rc=%s", strrc(rc));
        return rc;
    }
    return RC::SUCCESS;
}

void ParsedExprMapper::map_expr(ExprSqlNode *expr) 
{
    switch (expr->get_type())
    {
    case ExprSqlNodeType::Identifier: {
        auto id_expr = dynamic_cast<IdentifierExprSqlNode*>(expr);
        if (table_name_map_.count(id_expr->get_table_name()))
            id_expr->map_table_name(table_name_map_[id_expr->get_table_name()]);
        if (field_name_map_.count(id_expr->get_field_name()))
            id_expr->map_field_name(field_name_map_[id_expr->get_field_name()]);
    } break;

    case ExprSqlNodeType::NegativeArith: {
        auto neg_expr = dynamic_cast<NegativeArithExprSqlNode*>(expr);
        map_expr(neg_expr->sub_expr());
    } break;
    
    case ExprSqlNodeType::BinaryArith: {
        auto bin_expr = dynamic_cast<BinaryArithExprSqlNode*>(expr);
        map_expr(bin_expr->left_expr());
        map_expr(bin_expr->right_expr());
    } break;
    
    case ExprSqlNodeType::Compare: {
        auto cmp_expr = dynamic_cast<CompareExprSqlNode*>(expr);
        map_expr(cmp_expr->left_expr());
        map_expr(cmp_expr->right_expr());
    } break;
    
    case ExprSqlNodeType::PredNull: {
        auto null_expr = dynamic_cast<PredNullExprSqlNode*>(expr);
        map_expr(null_expr->sub_expr());
    } break;
    
    case ExprSqlNodeType::QuantifiedCompSubquery: {
        auto quantified_expr = dynamic_cast<QuantifiedCompSubqueryExprSqlNode*>(expr);
        map_expr(quantified_expr->sub_expr());
    } break;
    
    /* 剩下的写起来很麻烦，测到了再说 */
    default:
        break;
    }
}