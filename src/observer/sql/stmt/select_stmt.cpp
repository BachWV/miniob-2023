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
// Created by Wangyunlai on 2022/6/6.
//

#include <cassert>

#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/field/field.h"
#include "storage/table/table.h"
#include "sql/expr/parsed_expr.h"
#include "sql/expr/expression.h"
#include "sql/stmt/apply_stmt.h"

static void wildcard_fields(Table *table, std::vector<Field> &field_metas)
{
  const TableMeta &table_meta = table->table_meta();
  const int field_num = table_meta.field_num();
  for (int i = table_meta.sys_field_num(); i < field_num; i++) {
    field_metas.push_back(Field(table, table_meta.field(i)));
  }
}

// @param order_fields[out]
RC check_order_by_field(Db *db, const std::unordered_map<std::string, Table *>& table_map, const std::vector<Table *>& tables,
  const std::vector<OrderByAttrSqlNode>& order_sql_nodes, std::vector<FieldWithOrder>& order_fields)
{
  for(auto& node : order_sql_nodes){
    const RelAttrSqlNode &relation_attr = node.attr;

    if (!common::is_blank(relation_attr.relation_name.c_str())) {
      // 表名非空
      const char *table_name = relation_attr.relation_name.c_str();
      const char *field_name = relation_attr.attribute_name.c_str();
      
      // 常规情况，table.field
      auto iter = table_map.find(table_name);
      if (iter == table_map.end()) {
        LOG_WARN("no such table in from list: %s", table_name);
        return RC::SCHEMA_FIELD_MISSING;
      }
      
      Table *table = iter->second;
        const FieldMeta *field_meta = table->table_meta().field(field_name);
        if (nullptr == field_meta) {
          // 进行field合法性检查
          LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
          return RC::SCHEMA_FIELD_MISSING;
        }
        order_fields.push_back(FieldWithOrder(table, field_meta, node.is_asc));
    } else {
      // 没写table名，默认从第一个table中找
      if (tables.size() != 1) {
        LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }

      Table *table = tables[0];
      const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name.c_str());
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }
      order_fields.push_back(FieldWithOrder(table, field_meta, node.is_asc));
    }
  }
  return RC::SUCCESS;
}

RC SelectStmt::create(Db *db, ExprResolveContext *glob_ctx, const SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  StmtResolveContext current_where_resolve_ctx;  // 解析where子句的上下文

  // collect tables in `from` statement
  std::vector<Table *> tables;
  std::unordered_map<std::string, Table *> table_map;

  /* 做别名时记得在这里修改 */
  for (size_t i = 0; i < select_sql.relations.size(); i++) {
    const char *table_name = select_sql.relations[i].c_str();
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }

    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    tables.push_back(table);
    table_map.insert(std::pair<std::string, Table *>(table_name, table));

    current_where_resolve_ctx.add_table_to_namespace(table_name, table);
  }

  // collect query fields in `select` statement
  std::vector<Field> query_fields;
  for (int i = static_cast<int>(select_sql.attributes.size()) - 1; i >= 0; i--) {
    const RelAttrSqlNode &relation_attr = select_sql.attributes[i];

    if (common::is_blank(relation_attr.relation_name.c_str()) &&
        0 == strcmp(relation_attr.attribute_name.c_str(), "*")) {
          // 空.*
      for (Table *table : tables) {
        wildcard_fields(table, query_fields);
      }

    } else if (!common::is_blank(relation_attr.relation_name.c_str())) {
      // 表名非空
      const char *table_name = relation_attr.relation_name.c_str();
      const char *field_name = relation_attr.attribute_name.c_str();
      
      if (0 == strcmp(table_name, "*")) {
        // 只能是*.*
        if (0 != strcmp(field_name, "*")) {
          LOG_WARN("invalid field name while table is *. attr=%s", field_name);
          return RC::SCHEMA_FIELD_MISSING;
        }
        for (Table *table : tables) {
          wildcard_fields(table, query_fields);
        }
      } else {
        // 常规情况，table.field
        auto iter = table_map.find(table_name);
        if (iter == table_map.end()) {
          LOG_WARN("no such table in from list: %s", table_name);
          return RC::SCHEMA_FIELD_MISSING;
        }

        Table *table = iter->second;
        if (0 == strcmp(field_name, "*")) {
          // table.*
          wildcard_fields(table, query_fields);
        } else {
          const FieldMeta *field_meta = table->table_meta().field(field_name);
          if (nullptr == field_meta) {
            // 进行field合法性检查
            LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
            return RC::SCHEMA_FIELD_MISSING;
          }

          query_fields.push_back(Field(table, field_meta));
        }
      }
    } else {
      // 没写table名，默认从第一个table中找
      if (tables.size() != 1) {
        LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }

      Table *table = tables[0];
      const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name.c_str());
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }

      query_fields.push_back(Field(table, field_meta));
    }
  }

  LOG_INFO("got %d tables in from stmt and %d fields in query stmt", tables.size(), query_fields.size());

  RC rc = RC::SUCCESS;

  /* 开始解析where子句 */
  std::vector<std::unique_ptr<ApplyStmt>> apply_stmts;
  std::vector<std::unique_ptr<Expression>> where_exprs;
  glob_ctx->push_stmt_ctx(&current_where_resolve_ctx);

  // 解析每一个where条件(目前所有条件用AND连接)
  for (auto &where_expr_sql_node: select_sql.conditions)
  {
    ExprResolveResult where_resolve_res;
    rc = where_expr_sql_node->resolve(glob_ctx, &where_resolve_res);
    if (rc != RC::SUCCESS)
    {
      LOG_ERROR("resolve where expr failed. rc=%d", rc);
      return rc;
    }

    /* 对每一个where条件处理where中的子查询 */
    std::vector<SubQueryInfo>& sub_querys = where_resolve_res.get_subquerys_in_expr();
    for (auto& sub_query: sub_querys)
    {
      std::unique_ptr<ApplyStmt> sub_query_apply_stmt = sub_query.owns_apply_stmt();
      std::unordered_map<size_t, std::vector<CorrelateExpr*>>& correlate_exprs_in_subquery = sub_query.correlate_exprs();

      /* 
       * 将子查询的correlate_exprs中，引用本查询的相关表达式放进apply_stmt中，
       * 引用更上层的查询的相关表达式放到create的参数correlate_exprs中，视为本查询的相关表达式，继续向上传递
       * 即，如果本查询的子查询中出现了引用更上层的相关表达式，那么本查询也是上层的相关子查询
       */
      for (auto &[level, exprs]: correlate_exprs_in_subquery)
      {
        // 如果该相关表达式引用的是本层查询，直接加到apply算子中
        if (level == glob_ctx->current_level())
        {
          for (CorrelateExpr *epxr: exprs)
            sub_query_apply_stmt->add_correlate_expr(epxr);
        }

        // 否则，引用的是上层查询
        else
        {
          assert(level < glob_ctx->current_level());
          for (CorrelateExpr *expr: exprs)
            (*correlate_exprs)[level].push_back(expr);
        }
      }

      apply_stmts.emplace_back(std::move(sub_query_apply_stmt));  // 记录下生成好的applyStmt，之后保存到SelectStmt中
    }

    /* 
     * 处理where条件的相关表达式
     * where条件中如果出现了相关表达式，那么本查询也是一个相关子查询，把相关表达式添加到参数correlate_exprs中
     */
    std::unordered_map<size_t, std::vector<CorrelateExpr*>>& correlate_exprs_in_where = where_resolve_res.get_correlate_exprs();
    for (auto &[level, exprs]: correlate_exprs_in_where)
    {
      assert(level < glob_ctx->current_level());
      for (CorrelateExpr *expr: exprs)
        (*correlate_exprs)[level].push_back(expr);
    }

    where_exprs.emplace_back(where_resolve_res.owns_result_expr_tree());  // 记录生成的表达式树
  }
  
  glob_ctx->pop_stmt_ctx();

  // collect query fields in `order by` statement
  std::vector<FieldWithOrder> order_fields;
  rc = check_order_by_field(db, table_map, tables, select_sql.order_by_attrs, order_fields);
  if(rc != RC::SUCCESS){
    // log在check_order_by_field中打过了
    return rc;
  }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->tables_.swap(tables);
  select_stmt->query_fields_.swap(query_fields);
  select_stmt->order_fields_ = order_fields;
  select_stmt->where_exprs_.swap(where_exprs);
  select_stmt->sub_querys_in_where_.swap(apply_stmts);
  stmt = select_stmt;
  return RC::SUCCESS;
}
