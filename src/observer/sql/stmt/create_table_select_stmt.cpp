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

  // 当前create select只支持普通字段，不支持虚拟字段
  std::vector<SelectExprField>& select_exprs_fields = select_stmt->select_expr_fields();
  std::vector<AttrInfoSqlNode> infos;
  for(auto& e: select_exprs_fields){
    if(auto fwa = std::get_if<FieldWithAlias>(&e)){
      AttrInfoSqlNode info = fwa->field_.meta()->to_attr_info();
      if(!fwa->alias_.empty()){ // 别名的处理
        info.name = fwa->alias_;
      }
      infos.push_back(info);
      continue;
    }else{
      return RC::SQL_SYNTAX;
    }
  }

  CreateTableSelectStmt* cts_stmt = new CreateTableSelectStmt();
  cts_stmt->created_table_infos_ = infos;
  cts_stmt->original_table_stmt_ =select_stmt;
  cts_stmt->created_table_name_ = sql_node.created_table_name_;
  stmt = cts_stmt;
  return RC::SUCCESS;
}