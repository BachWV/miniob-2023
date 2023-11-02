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
#include "sql/view/view.h"

#include <unordered_set>
#include <variant>

using TableOrView = std::variant<Table*, View*>;

static void wildcard_col_names(const TableOrView &table_src, std::vector<std::string>& column_names)
{
  if (auto p_table = std::get_if<Table*>(&table_src))
  {
    const Table *table = *p_table;
    const TableMeta &table_meta = table->table_meta();
    const int field_num = table_meta.field_num();
    for (int i = table_meta.sys_field_num(); i < field_num; i++) {
      const FieldMeta *field_meta = table_meta.field(i);
      column_names.emplace_back(field_meta->name());
    }
  }
  else if (auto p_view = std::get_if<View*>(&table_src))
  {
    const std::vector<ViewColumnAttr> &col_name_infos = (*p_view)->view_columns();
    size_t col_num = col_name_infos.size();
    for (size_t i = 0; i < col_num; ++i)
      column_names.emplace_back(col_name_infos[i].column_name);
  }
}

InsertStmt::InsertStmt(Table *table, std::vector<std::vector<Value> >value_rows)
    : table_(table), value_rows_(value_rows)
{}

RC InsertStmt::create(Db *db, InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || inserts.value_rows.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d",
        db, table_name, static_cast<int>(inserts.value_rows.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr != table) {
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
    }

    // everything alright
    stmt = new InsertStmt(table, inserts.value_rows);
    return RC::SUCCESS;
  }

  else {  // 尝试在视图中插入
    View *view = db->find_view(table_name);
    if (view == nullptr) {
      LOG_WARN("no such table or view.");
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    if (!view->insertable()) {
      LOG_WARN("view %s can't insert.", table_name);
      return RC::VIEW_CANNOT_INSERT;
    }

    /* 检查：所有行的元素个数必须和视图定义相等 */
    const std::vector<ViewColumnAttr> &view_columns = view->view_columns();
    for(const std::vector<Value> &values : inserts.value_rows) {
      // check the fields number
      const int value_num  = static_cast<int>(values.size());
      if (view_columns.size() != value_num) {
        LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, view_columns.size());
        return RC::SCHEMA_FIELD_MISSING;
      }
    }

    /* 检查：所有列必须是引用同一个表的基础field，且没有重复field */
    std::unordered_set<std::string> tables_set, fields_set;
    for (const ViewColumnAttr &view_column : view_columns) {
      if (!view_column.is_field_identifier) {
        LOG_WARN("view %s can't insert because column %s is not field identifier.", table_name, view_column.column_name.c_str());
        return RC::VIEW_CANNOT_INSERT;
      }
      tables_set.insert(view_column.related_table_name);
      if (tables_set.size() > 1) {
        LOG_WARN("view %s can't insert because refered to multi tables.", table_name);
        return RC::VIEW_CANNOT_INSERT;
      }
      if (fields_set.count(view_column.field_name)) {
        LOG_WARN("view %s can't insert because refered to repeat field.", table_name);
        return RC::VIEW_CANNOT_INSERT;
      }
      fields_set.insert(view_column.field_name);
    }

    const std::string target_table_name = view_columns.at(0).related_table_name;
    TableOrView target_to_insert;
    Table *target_table = db->find_table(target_table_name.c_str());
    if (target_table != nullptr) {
      target_to_insert = target_table;
    }
    else {
      View *target_view = db->find_view(target_table_name.c_str());
      if (target_view == nullptr) {
        LOG_WARN("no such table or view.");
        return RC::SCHEMA_TABLE_NOT_EXIST;
      }
      target_to_insert = target_view;
    }

    /* 构造完整的插入数据，视图只是投影，没有投影的字段值设为null */
    std::vector<std::string> target_column_names;
    wildcard_col_names(target_to_insert, target_column_names);
    std::vector<std::vector<Value>> value_rows_to_insert;

    /* 对于每一个待插入的行，构造完整的行数据 */
    for (size_t value_row = 0; value_row < inserts.value_rows.size(); ++value_row)
    {
      std::vector<Value> cur_value_row;

      /* 按原表顺序，遍历原表的每一列 */
      for (size_t idx = 0; idx < target_column_names.size(); ++idx) 
      {
        // 找到传入的行中，用于插入原表第idx列的值，通过列名比较进行查找
        bool found = false;
        for (int col_id = 0; col_id < view_columns.size(); ++col_id) {
          if (view_columns[col_id].field_name == target_column_names[idx]) {
            cur_value_row.push_back(inserts.value_rows[value_row][col_id]);
            found = true;
            break;
          }
        }

        // 如果原表第idx列不在视图中，则设置为空值
        if (!found) {
          Value val;
          val.set_null();
          cur_value_row.push_back(val);
        }
      }

      value_rows_to_insert.emplace_back(std::move(cur_value_row));
    }

    /* 构造新的InsertSqlNode，递归调用以插入视图引用的表(他可能是基表，或者还是个视图) */
    InsertSqlNode new_inserts;
    new_inserts.relation_name = target_table_name;
    new_inserts.value_rows = std::move(value_rows_to_insert);
    return create(db, new_inserts, stmt);
  }
}
