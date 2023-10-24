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
#include "sql/parser/parse_defs.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/field/field.h"
#include "storage/field/field_meta.h"
#include "storage/table/table.h"
#include "sql/expr/parsed_expr.h"
#include "sql/expr/expression.h"
#include "sql/stmt/apply_stmt.h"
#include <cstring>
#include <functional>
#include <unordered_set>

static void wildcard_fields(Table *table, std::vector<SelectExprField>& select_expr_fields)
{
  const TableMeta &table_meta = table->table_meta();
  const int field_num = table_meta.field_num();
  for (int i = table_meta.sys_field_num(); i < field_num; i++) {
    select_expr_fields.push_back(Field(table, table_meta.field(i)));
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

// 只检查具名的字段(如id，name等)，带有"*"不会通过check
// 成功返回RC::success，并返回构建好的field；失败返回RC::SCHEMA_FIELD_MISSING
RC resolve_common_field(Db *db, const std::unordered_map<std::string, Table *>& table_map, const std::vector<Table *>& tables,
  const RelAttrSqlNode& relation_attr, Field& field){
    
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
        field = Field(table, field_meta);
        return RC::SUCCESS;
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
      field = Field(table, field_meta);
      return RC::SUCCESS;
    }
  return RC::SCHEMA_FIELD_MISSING;
}

RC SelectStmt::create(Db *db, ExprResolveContext *glob_ctx, const SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  StmtResolveContext current_where_resolve_ctx;  // 解析where子句的上下文
  StmtResolveContext having_resolve_ctx;  // 解析having子句的上下文

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
    having_resolve_ctx.add_table_to_namespace(table_name, table);
  }

  // collect fields in `group by` statement
  std::vector<Field> group_fields;
  for(const auto& rel_attr: select_sql.group_by_attrs){
    Field group_by_field;
    auto rc = resolve_common_field(db, table_map, tables, rel_attr, group_by_field);
    if(rc != RC::SUCCESS){
      return rc;
    }
    group_fields.push_back(group_by_field);
  }

  // collect select_expr in `select` statement
  // std::visit只能重载()传递单参数，重载lambda对参数捕获又太麻烦，怕环境编译器不支持太高级的特性，选择直接if else
  std::vector<std::string> agg_functions_names;
  std::vector<SelectExprField> select_expr_fields;
  std::unordered_set<FieldIdentifier, FieldIdentifierHash> common_fields_set;
  bool has_agg = false;

  for(const auto& select_expr_field: select_sql.select_exprs){
    if(auto relation_attr = get_if<RelAttrSqlNode>(&select_expr_field)){

      // 这里的代码Copy之前的验证Field字段的代码
      if (common::is_blank(relation_attr->relation_name.c_str()) &&
          0 == strcmp(relation_attr->attribute_name.c_str(), "*")) {

        // 空.*
        for (Table *table : tables) {
          wildcard_fields(table, select_expr_fields);
        }// next loop

      } else if (!common::is_blank(relation_attr->relation_name.c_str())) {

        // 表名非空
        const char *table_name = relation_attr->relation_name.c_str();
        const char *field_name = relation_attr->attribute_name.c_str();
        
        if (0 == strcmp(table_name, "*")) {
          // 只能是*.*
          if (0 != strcmp(field_name, "*")) {
            LOG_WARN("invalid field name while table is *. attr=%s", field_name);
            return RC::SCHEMA_FIELD_MISSING;
          }
          for (Table *table : tables) {
            wildcard_fields(table, select_expr_fields);
          }// next loop

        } else {
          auto iter = table_map.find(table_name);
          if (iter == table_map.end()) {
            LOG_WARN("no such table in from list: %s", table_name);
            return RC::SCHEMA_FIELD_MISSING;
          }

          Table *table = iter->second;
          if (0 == strcmp(field_name, "*")) {
            // table.*
            wildcard_fields(table, select_expr_fields);
            // next loop

          } else {
            // 常规情况，table.field
            const FieldMeta *field_meta = table->table_meta().field(field_name);
            if (nullptr == field_meta) {
              LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
              return RC::SCHEMA_FIELD_MISSING;
            }

            auto common_field = Field(table, field_meta);
            select_expr_fields.push_back(common_field);
            common_fields_set.insert(FieldIdentifier(table->name(), field_meta->name()));
          }
        }

      } else {
        // 没写table名，默认从第一个table中找
        if (tables.size() != 1) {
          LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr->attribute_name.c_str());
          return RC::SCHEMA_FIELD_MISSING;
        }

        Table *table = tables[0];
        const FieldMeta *field_meta = table->table_meta().field(relation_attr->attribute_name.c_str());
        if (nullptr == field_meta) {
          LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr->attribute_name.c_str());
          return RC::SCHEMA_FIELD_MISSING;
        }

        auto common_field = Field(table, field_meta);
        select_expr_fields.push_back(common_field);
        common_fields_set.insert(FieldIdentifier(table->name(), field_meta->name()));
      }

    }else if(auto agg_sql_node = get_if<AggregateFuncSqlNode>(&select_expr_field)){
      has_agg = true;
      agg_functions_names.push_back(agg_sql_node->name);
      Field agg_field;
      auto rc = resolve_common_field(db, table_map, tables, agg_sql_node->attr, agg_field);
      if(rc != RC::SUCCESS){
        // 若为COUNT(*)的处理。AGG算子中要手动释放agg_field的内存
        if(agg_sql_node->agg_op == AGG_COUNT && 0 == strcmp(agg_sql_node->attr.attribute_name.c_str(), "*") ){
          FieldMeta* meta =  new FieldMeta("*", INTS, 1,1,false, false);
          assert(tables[0]);
          agg_field = Field(tables[0], meta);
        }else{
          return rc;
        }
      }
      bool with_table_name = tables.size() > 1;
      select_expr_fields.push_back(FieldsWithGroupBy(agg_field, group_fields, agg_sql_node->agg_op, with_table_name));
    }else{
      // 出问题了，debug模式下直接kill
      assert(0);
      return RC::UNIMPLENMENT;
    }
  }

  // check aggregation validate: 如果存在agg，那么非agg的字段（common_fields_set）一定在gb中出现
  if(has_agg){
    // 非agg的字段和group by字段数一定相等，要不都是0。
    if(common_fields_set.size() != group_fields.size()){
      return RC::SQL_SYNTAX;
    }

    // 若gf的字段在cfs中不存在，那么一定有错
    for(auto const& field: group_fields){
      auto fi = FieldIdentifier(field.table_name(), field.field_name());
      if(auto it = common_fields_set.find(fi); it == common_fields_set.end()){
        return RC::SQL_SYNTAX;
      }
    }
  }


  RC rc = RC::SUCCESS;
  /* 开始解析HAVING子句
   * 把Having里可能出现的列名，调用having_resolve_ctx.add_other_column_name传入
   */
  for(auto& str: agg_functions_names){
    having_resolve_ctx.add_other_column_name(str);
  }

  std::vector<std::unique_ptr<Expression>> having_exprs;
  glob_ctx->push_stmt_ctx(&having_resolve_ctx);
  for (auto &having_expr: select_sql.having_attrs)
  {
    ExprResolveResult having_resolve_result;
    rc = having_expr->resolve(glob_ctx, &having_resolve_result);
    if (rc != RC::SUCCESS)
    {
      LOG_WARN("resolve having expr failed. rc=%d", strrc(rc));
      return rc;
    }

    assert(having_resolve_result.get_subquerys_in_expr().empty());
    assert(having_resolve_result.get_correlate_exprs().empty());

    having_exprs.emplace_back(having_resolve_result.owns_result_expr_tree());

    /* TODO: 处理HAVING子句中，不存在于select的列中的聚集函数，使用having_resolve_result.get_agg_expr_infos()接口 */
  }
  glob_ctx->pop_stmt_ctx();

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
      LOG_WARN("resolve where expr failed. rc=%d", strrc(rc));
      return rc;
    }

    /* 拿到where中的子查询 */
    std::vector<std::unique_ptr<ApplyStmt>>& subquerys_in_where = where_resolve_res.get_subquerys_in_expr();
    for (auto &subquery: subquerys_in_where)
      apply_stmts.emplace_back(std::move(subquery));

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
  select_stmt->select_expr_fields_.swap(select_expr_fields);
  select_stmt->group_by_field_.swap(group_fields);
  select_stmt->order_fields_ = order_fields;
  select_stmt->where_exprs_.swap(where_exprs);
  select_stmt->sub_querys_in_where_.swap(apply_stmts);
  select_stmt->having_exprs_.swap(having_exprs);
  stmt = select_stmt;
  return RC::SUCCESS;
}
