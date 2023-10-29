#include "create_table_select_stmt.h"
#include "common/rc.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/expression.h"
#include "sql/expr/parsed_expr.h"
#include <vector>


RC CreateTableSelectStmt::create(Db *db, CreateTableSelectSqlNode &sql_node, Stmt *&stmt){
  SelectStmt* select_stmt; 
  RC rc = RC::SUCCESS;

  ExprResolveContext select_ctx;
  std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_in_select;
  Stmt *stmt_tmp = nullptr;
  rc = SelectStmt::create(db, &select_ctx, sql_node.selected_sql_node_, stmt_tmp, &correlate_in_select);
  HANDLE_RC(rc);
  assert(correlate_in_select.empty());

  select_stmt = dynamic_cast<SelectStmt*>(stmt_tmp);

  CreateTableSelectStmt* cts_stmt = new CreateTableSelectStmt();
  cts_stmt->created_table_infos_ = select_stmt->column_attrs();
  cts_stmt->original_table_stmt_ =select_stmt;
  cts_stmt->created_table_name_ = sql_node.created_table_name_;
  stmt = cts_stmt;
  // return RC::SUCCESS;
  return RC::SQL_SYNTAX;
}