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

#include "sql/stmt/select_stmt.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/field/field.h"
#include "storage/field/field_meta.h"
#include "storage/table/table.h"
#include <cstring>

SelectStmt::~SelectStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

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

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  // collect tables in `from` statement
  std::vector<Table *> tables;
  std::unordered_map<std::string, Table *> table_map;
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
  std::vector<SelectExprField> select_expr_fields;
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

            select_expr_fields.push_back(Field(table, field_meta));
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

        select_expr_fields.push_back(Field(table, field_meta));
      }

    }else if(auto agg_sql_node = get_if<AggregateFuncSqlNode>(&select_expr_field)){
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

  LOG_INFO("got %d tables in from stmt and %d fields in query stmt", tables.size(), select_expr_fields.size());

  Table *default_table = nullptr;
  if (tables.size() == 1) {
    default_table = tables[0];
  }

  // create filter statement in `where` statement
  FilterStmt *filter_stmt = nullptr;
  RC rc = FilterStmt::create(db,
      default_table,
      &table_map,
      select_sql.conditions.data(),
      static_cast<int>(select_sql.conditions.size()),
      filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("cannot construct filter stmt");
    return rc;
  }

  // collect query fields in `order by` statement
  std::vector<FieldWithOrder> order_fields;
  rc = check_order_by_field(db, table_map, tables, select_sql.order_by_attrs, order_fields);
  if(rc != RC::SUCCESS){
    // log在check_order_by_field中打过了
    return rc;
  }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();
  // TODO add expression copy
  select_stmt->tables_.swap(tables);
  select_stmt->select_expr_fields_.swap(select_expr_fields);
  select_stmt->group_by_field_.swap(group_fields);
  select_stmt->filter_stmt_ = filter_stmt;
  select_stmt->order_fields_ = order_fields;
  stmt = select_stmt;
  return RC::SUCCESS;
}
