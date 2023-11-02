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
#include "sql/stmt/select_stmt.h"
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

RC UpdateStmt::create(Db *db, UpdateSqlNode &update_sql, Stmt *&stmt)
{
  const char *table_name = update_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }
  

    // check whether the table exists
  Table *table = db->find_table(table_name);

  // 插入的是基表，正常处理
  if (nullptr != table) {
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

  // 否则，检查是否是插入视图
  else {
    View *view = db->find_view(table_name);
    if (view == nullptr) {
      LOG_WARN("no such table or view.");
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    if (!view->updatable()) {
      LOG_WARN("view %s is not updatable.", view->name().c_str());
      return RC::VIEW_CANNOT_UPDATE;
    }

    /* 工具类：将update语句中，引用本视图field的表达式映射到待更新的基表和基表field */
    ParsedExprMapper expr_mapper;

    ExprResolveContext expr_ctx;
    StmtResolveContext update_ctx;

    /* 
     * 检查每一个待更新的列，这些列满足：
     * 1) 在视图中是一个简单的基表field引用
     * 2) 必须指向同一个基表
     * 如果满足条件，把set左侧的列名修改成基表对应的列名
     */
    std::string target_table_name;
    for (auto &set_value_def: update_sql.set_value_list)
    {
      std::string &col_name = set_value_def.attr_name;

      /* 在视图定义中查找这个列的属性 */
      const std::vector<ViewColumnAttr> &view_columns = view->view_columns();
      bool found = false;
      for (auto &view_col: view_columns)
      {
        if (view_col.column_name == col_name)
        {
          if (!view_col.is_field_identifier)
          {
            LOG_WARN("view %s is not updatable because column is not field identifier.", view->name().c_str());
            return RC::VIEW_CANNOT_UPDATE;
          }

          if (target_table_name.empty())
            target_table_name = view_col.related_table_name;
          else if (target_table_name != view_col.related_table_name)
          {
            LOG_WARN("view %s is not updatable because column refers to multi tables.", view->name().c_str());
            return RC::VIEW_CANNOT_UPDATE;
          }

          found = true;
          col_name = view_col.field_name;
          break;
        }
      }

      if (!found)
      {
        LOG_WARN("no such field, view name=%s, col_name = %s", view->name().c_str(), col_name.c_str());
        return RC::VIEW_CANNOT_UPDATE;
      }
    }
    expr_mapper.add_table_map(view->name(), target_table_name);
    update_sql.relation_name = target_table_name;  // 将要更新的表名修改成基表名

    /*
     * 目前，set右侧和where条件中，只允许出现引用待更新列所属表的identifier，否则不可更新
     * 把引用待更新表的视图列添加到expr_ctx的namespace中
     * 这里构造完整namespace太麻烦了，所以namespace中，视图列的Identifier无效，解析后的表达式不能求值
     * 我们也不会对表达式求值，只需要知道是否合法即可
     * 
     * 同时把这些列添加到expr_mapper中，map到待更新表中的列名
     */
    for (auto &view_col: view->view_columns())
    {
      if (view_col.is_field_identifier && view_col.related_table_name == target_table_name)
      {
        update_ctx.add_view_column(view->name(), view_col.column_name, FieldIdentifier(), ExprValueAttr());
        expr_mapper.add_field_map(view_col.column_name, view_col.field_name);
      }
    }
    expr_ctx.push_stmt_ctx(&update_ctx);

    /* 分别检查并重写set，where中的表达式 */
    for (auto &set_sql_node: update_sql.set_value_list)
    {
      ExprResolveResult result;
      ExprSqlNode *expr_sql_node = set_sql_node.rhs_expr.get();
      RC rc = expr_sql_node->resolve(&expr_ctx, &result);
      if (rc != RC::SUCCESS)
      {
        LOG_WARN("expr in view update refers to non-update columns.");
        return rc;
      }
      expr_mapper.map_expr(set_sql_node.rhs_expr.get());
    }
    for (auto &cond: update_sql.conditions.exprs)
    {
      ExprResolveResult result;
      ExprSqlNode *expr_sql_node = cond.get();
      RC rc = expr_sql_node->resolve(&expr_ctx, &result);
      if (rc != RC::SUCCESS)  // 如果测试时允许，那么这里就忽略掉该表达式
      {
        LOG_WARN("expr in view update refers to non-update columns.");
        return rc;
      }
      expr_mapper.map_expr(expr_sql_node);
    }

    /* 现在已经把对视图的更新转换成对基表的更新了，递归调用 */
    return create(db, update_sql, stmt);
  }
}
