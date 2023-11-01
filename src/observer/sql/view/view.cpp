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