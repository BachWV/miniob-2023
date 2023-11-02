#include "sql/stmt/view_stmt.h"
#include "storage/db/db.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/parsed_expr.h"
#include "sql/expr/expression.h"
#include "sql/stmt/apply_stmt.h"
#include "sql/view/view.h"

#include <unordered_set>

ViewStmt::ViewStmt(std::unique_ptr<View> view)
{
    view_ = std::move(view);
}

std::unique_ptr<View> ViewStmt::owns_view()
{
    return std::move(view_);
}

RC ViewStmt::create(Db *db, CreateViewSqlNode &view_node, Stmt *&stmt)
{
    RC rc = RC::SUCCESS;
    SelectSqlNode &select_sql_node = view_node.select_def->selection;
    Stmt *select_stmt_base = nullptr;

    /* 通过select的resolve验证这个查询是否合法 */
    ExprResolveContext expr_ctx;
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlates_in_select;
    rc = SelectStmt::create(db, &expr_ctx, select_sql_node, select_stmt_base, &correlates_in_select);
    std::unique_ptr<SelectStmt> select_stmt(dynamic_cast<SelectStmt*>(select_stmt_base));
    if (rc != RC::SUCCESS || !correlates_in_select.empty()) {
        LOG_WARN("select stmt in create view is invalid.");
        return RC::INVALID_ARGUMENT;
    }

    /* 创建视图的元信息 */
    size_t view_explicit_col_size = view_node.view_column_names.size();
    if (view_explicit_col_size != 0 && view_explicit_col_size != select_stmt->column_attrs().size()) {
        LOG_WARN("view column size is not equal to select column size.");
        return RC::INVALID_ARGUMENT;
    }

    bool insertable = true, deletable = true, updatable = true;
    if (select_stmt->table_map().empty() || 
        !select_stmt->get_group_by_fields().empty() ||
        select_stmt->has_having_clause() ||
        !select_stmt->fetch_sub_querys_in_where().empty())  // select不满足条件，视图直接不可更新
    {
        insertable = false;
        deletable = false;
        updatable = false;
    }

    const std::vector<SelectColumnInfo> &column_exprs = select_stmt->select_expr_fields();
    const std::vector<AttrInfoSqlNode> &columns_in_select = select_stmt->column_attrs();
    const std::vector<std::pair<bool, std::string>> &is_simple_field_ref_vec = select_stmt->is_simple_field_ident_vec();
    std::vector<ViewColumnAttr> view_col_infos;

    for (size_t i = 0; i < columns_in_select.size(); ++i)
    {
        ViewColumnAttr cur_col_info;
        if (is_simple_field_ref_vec.at(i).first == false)  // 如果列不是简单的field引用，则不可插入，不可删除
        {
            cur_col_info.is_field_identifier = false;
            insertable = deletable = updatable = false;
        }
        else
        {
            cur_col_info.is_field_identifier = true;
            Expression *col_expr = column_exprs.at(i).expr_.get();
            std::vector<std::string> col_referred_table = col_expr->referred_tables();
            if (col_referred_table.size() != 1) {
                LOG_WARN("col expr referred table count should be one when it's a field identifier, but here is %u.",  
                    col_referred_table.size());
                return RC::INTERNAL;
            }
            cur_col_info.related_table_name = col_referred_table[0];
            cur_col_info.field_name = is_simple_field_ref_vec.at(i).second;
        }
        cur_col_info.column_name = view_node.view_column_names.empty() ? columns_in_select.at(i).name : 
            view_node.view_column_names.at(i);
        view_col_infos.emplace_back(std::move(cur_col_info));
    }

    auto view_ = std::make_unique<View>(view_node.view_name, std::move(view_col_infos), 
        std::make_unique<SelectSqlNode>(std::move(select_sql_node)), insertable, deletable, updatable);
    stmt = new ViewStmt(std::move(view_));
    return RC::SUCCESS;
}