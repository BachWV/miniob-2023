/* Copyright (c) 2021OceanBase and/or its affiliates. All rights reserved.
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

#include "sql/stmt/insert_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

InsertStmt::InsertStmt(Table *table, std::vector<std::vector<Value> >value_rows)
    : table_(table), value_rows_(value_rows)
{}

RC InsertStmt::create(Db *db, const InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || inserts.value_rows.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d",
        db, table_name, static_cast<int>(inserts.value_rows.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    // 发生错误后，RC的返回值由谁接受
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  const TableMeta &table_meta = table->table_meta();
      // 注意我们要减去sys_field
  const int field_num = table_meta.field_num() - table_meta.sys_field_num();

  for(const std::vector<Value> &values : inserts.value_rows) {
    // check the fields number
    const int value_num  = static_cast<int>(values.size());
    if (field_num != value_num) {
      LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, field_num);
      return RC::SCHEMA_FIELD_MISSING;
    }

    // check fields type
    // make_record时check并进行类型转换
  //   const int sys_field_num = table_meta.sys_field_num();
  //   for (int i = 0; i < value_num; i++) {
  //     // 找的时候要加上sys_field
  //     const FieldMeta *field_meta = table_meta.field(i + sys_field_num);
  //     const AttrType   field_type = field_meta->type();
  //     const AttrType   value_type = values[i].attr_type();
  //     if (field_type == TEXTS && value_type == CHARS)
  //       continue;
  //     if (field_type != value_type && value_type != NULL_TYPE) {  // TODO try to convert the value type to field type
  //       LOG_WARN("field type mismatch. table=%s, field=%s, field type=%d, value_type=%d",
  //         table_name, field_meta->name(), field_type, value_type);
  //       return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  //     }
  //   }
  }

  // everything alright
  stmt = new InsertStmt(table, inserts.value_rows);
  return RC::SUCCESS;
}
