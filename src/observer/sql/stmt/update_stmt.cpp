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
#include "sql/stmt/update_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/expr/parsed_expr.h"
#include "sql/stmt/apply_stmt.h"
#include "sql/expr/expression.h"

UpdateStmt::UpdateStmt(Table *table, std::unordered_map<int, std::unique_ptr<Expression>> &&value_list,
  std::vector<std::unique_ptr<ApplyStmt>> &&subquerys_in_set_stmts, std::unique_ptr<ConjunctionExpr> cond_exprs)
    : table_(table), value_list_(std::move(value_list)), subquerys_in_set_stmts_(std::move(subquerys_in_set_stmts)),
     cond_exprs_(std::move(cond_exprs)) {}

std::unique_ptr<ConjunctionExpr> UpdateStmt::fetch_cond_exprs()
{
  return std::move(cond_exprs_);
}

std::unordered_map<int, std::unique_ptr<Expression>> &UpdateStmt::fetch_value_list()
{
  return value_list_;
}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt)
{
  const char *table_name = update_sql.relation_name.c_str();
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
  // check the fields name
  const TableMeta &table_meta = table->table_meta();

  // 解析每一个set右侧的表达式
  RC rc = RC::SUCCESS;
  ExprResolveContext expr_ctx;
  StmtResolveContext update_ctx;
  update_ctx.add_table_to_namespace(table_name, table);
  expr_ctx.push_stmt_ctx(&update_ctx);

  std::unordered_map<int, std::unique_ptr<Expression>> set_expr_list;
  std::vector<std::unique_ptr<ApplyStmt>> subquerys_in_set_expr;

  for(auto &set_value :update_sql.set_value_list){
    int index;
    bool field_exist=false;
    for (int i=0;i<table_meta.field_num();i++) {
      if (0 == strcmp(table_meta.field_metas()->at(i).name(),set_value.attr_name.c_str())) {
        index=i;
        field_exist=true;
        break;
      }
    }

    if(!field_exist){
      LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), set_value.attr_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }

    ExprResolveResult res;
    rc = set_value.rhs_expr->resolve(&expr_ctx, &res);
    if (rc != RC::SUCCESS)
    {
      LOG_WARN("failed to resolve expr in set statement, rc=%s", strrc(rc));
      return rc;
    }
    set_expr_list.emplace(index, res.owns_result_expr_tree());

    // 处理set右边表达式中的子查询
    std::vector<std::unique_ptr<ApplyStmt>> &sub_querys = res.get_subquerys_in_expr();
    subquerys_in_set_expr.insert(subquerys_in_set_expr.end(), std::make_move_iterator(sub_querys.begin()),
                                 std::make_move_iterator(sub_querys.end()));
    if (!res.get_correlate_exprs().empty() || !res.get_agg_expr_infos().empty())
    {
      LOG_WARN("set statement has unexpected correlate expr or agg expr.");
      return RC::UNIMPLENMENT;
    }
  }

  // 处理where子句
  std::vector<std::unique_ptr<Expression>> cond_exprs;
  for (auto &cond_expr_sql_node: update_sql.conditions.exprs)
  {
    ExprResolveResult result;
    rc = cond_expr_sql_node->resolve(&expr_ctx, &result);
    if (rc != RC::SUCCESS)
      return rc;

    /* 目前update中不会有子查询的情况，所以只用result中的expr树就行了 */
    cond_exprs.emplace_back(result.owns_result_expr_tree());
    if (!result.get_subquerys_in_expr().empty())
    {
      LOG_WARN("update statement does not support subquery in where clause now.");
      return RC::UNIMPLENMENT;
    }
  }
  
  expr_ctx.pop_stmt_ctx();

  //everything is ok, create the update statement
  auto conjunction_expr = std::make_unique<ConjunctionExpr>(
    update_sql.conditions.type == Conditions::ConjunctionType::AND ? ConjunctionExpr::Type::AND : ConjunctionExpr::Type::OR,
    cond_exprs);
  stmt = new UpdateStmt(table, std::move(set_expr_list), std::move(subquerys_in_set_expr), std::move(conjunction_expr));
  return rc;    

}
