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
// Created by Wangyunlai on 2022/07/05.
//

#pragma once

#include "storage/table/table.h"
#include "storage/field/field_meta.h"
#include "sql/expr/tuple_cell.h"

/**
 * @brief 字段
 * 
 */
class Field 
{
public:
  Field() = default;
  Field(const Table *table, const FieldMeta *field) : table_(table), field_(field)
  {}
  Field(const Field &) = default;

  const Table *table() const
  {
    return table_;
  }
  const FieldMeta *meta() const
  {
    return field_;
  }

  AttrType attr_type() const
  {
    return field_->type();
  }

  const char *table_name() const
  {
    return table_->name();
  }
  const char *field_name() const
  {
    return field_->name();
  }

  void set_table(const Table *table)
  {
    this->table_ = table;
  }
  void set_field(const FieldMeta *field)
  {
    this->field_ = field;
  }

  void set_int(Record &record, int value);
  int  get_int(const Record &record);

  const char *get_data(const Record &record);

private:
  const Table *table_ = nullptr;
  const FieldMeta *field_ = nullptr;
};

// 用于排序
class FieldWithOrder{
public:
  FieldWithOrder(const Table *table, const FieldMeta *field, bool is_asc) : field(Field(table, field)), is_asc_(is_asc){}
  Field field;
  bool get_is_acs(){return is_asc_;}
private:
  bool is_asc_;
};

// 字段标识符，使用表名+列名/或字段名(对于聚集函数列、表达式别名）来标识一个Tuple中的字段
class FieldIdentifier 
{
public:
  FieldIdentifier(const std::string &table_name, const std::string &field_name) 
    : table_name_(table_name), field_name_(field_name) {}
  FieldIdentifier(const std::string &colunm_name) : field_name_(colunm_name) {}

  const std::string& table_name() const {
    return table_name_;
  }

  const std::string &field_name() const {
    return field_name_;
  }

  TupleCellSpec to_tuple_cell_spec() const {
    return TupleCellSpec(table_name_.c_str(), field_name_.c_str());
  }

private:

  /* 若table_name_不为空，则identifier是表的列名。否则，field_name_表示虚拟列名(表达式/聚集/用于替代子查询表达式的名字) */
  std::string table_name_, field_name_;
  bool is_table_field_identifier;
};