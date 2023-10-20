/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "common/log/log.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/expr/parsed_expr.h"
#include "sql/stmt/apply_stmt.h"

DeleteStmt::DeleteStmt(Table *table, std::vector<std::unique_ptr<Expression>> &&cond_exprs) 
  : table_(table), cond_exprs_(std::move(cond_exprs))
{}

RC DeleteStmt::create(Db *db, const DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  const char *table_name = delete_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // 处理where子句
  ExprResolveContext expr_ctx;
  StmtResolveContext delete_ctx;

  RC rc = RC::SUCCESS;
  delete_ctx.add_table_to_namespace(table_name, table);
  expr_ctx.push_stmt_ctx(&delete_ctx);

  std::vector<std::unique_ptr<Expression>> cond_exprs;
  for (auto &cond_expr_sql_node: delete_sql.conditions)
  {
    ExprResolveResult result;
    rc = cond_expr_sql_node->resolve(&expr_ctx, &result);
    if (rc != RC::SUCCESS)
      return rc;

    /* 目前delete中不会有子查询的情况，所以只用result中的expr树就行了 */
    cond_exprs.emplace_back(result.owns_result_expr_tree());
  }

  expr_ctx.pop_stmt_ctx();

  stmt = new DeleteStmt(table, std::move(cond_exprs));
  return rc;
}
