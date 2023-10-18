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

UpdateStmt::UpdateStmt(Table *table,std::unordered_map<int,Value> value_list,FilterStmt *filter_stmt)
    : table_(table), value_list_(value_list),filter_stmt_(filter_stmt)
{}

UpdateStmt::~UpdateStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt)
{
  // TODO
  //stmt = nullptr;
  //return RC::INTERNAL;
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

  std::unordered_map<int, Value> value_list;
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
    value_list.insert(std::pair<int, Value>(index,set_value.value));
    if(!field_exist){
      LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), set_value.attr_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    const FieldMeta &field_meta = table_meta.field_metas()->at(index);
    const AttrType &field_type = field_meta.type();
    const AttrType &value_type = set_value.value.attr_type();
    if(field_type != value_type){
      LOG_WARN("field type mismatch. table=%s, field=%s, field type=%d, value_type=%d",
               table->name(), field_meta.name(), field_type, value_type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }

  }


  //filter is the where clause
  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));

  FilterStmt *filter_stmt = nullptr;
  RC rc = FilterStmt::create(
      db, table, &table_map, update_sql.conditions.data(), static_cast<int>(update_sql.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  

  //everything is ok, create the update statement
  stmt = new UpdateStmt(table, value_list, filter_stmt);
  return rc;    

}
