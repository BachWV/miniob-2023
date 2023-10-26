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

#include "sql/parser/value.h"
#include "storage/table/table.h"
#include "storage/field/field_meta.h"
#include "sql/expr/tuple_cell.h"
#include "common/log/log.h"

enum AggregateOp;

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
  FieldWithOrder(Field f, bool is_asc): field(f), is_asc_(is_asc){}
  Field field;
  bool get_is_acs(){return is_asc_;}
private:
  bool is_asc_;
};

// 字段标识符，使用表名+列名/或字段名(对于聚集函数列、表达式别名）来标识一个Tuple中的字段
class FieldIdentifier 
{
public:
  FieldIdentifier() = default;
  FieldIdentifier(std::string table_name, std::string field_name, std::string alias)
    :table_name_(table_name), field_name_(field_name), alias_(alias){}
  FieldIdentifier(const std::string &table_name, const std::string &field_name) 
    : table_name_(table_name), field_name_(field_name) {}
  FieldIdentifier(const std::string &colunm_name) : field_name_(colunm_name) {}

  const std::string& table_name() const {
    return table_name_;
  }

  const std::string &field_name() const {
    return field_name_;
  }

  const std::string &alias() const {
    return alias_;
  }

  TupleCellSpec to_tuple_cell_spec() const {
    return TupleCellSpec(table_name_.c_str(), field_name_.c_str());
  }

  bool operator==(const FieldIdentifier& fi) const {
    return (table_name_ == fi.table_name_) && (field_name_ == fi.field_name_);
  }

private:

  /* 若table_name_不为空，则identifier是表的列名。否则，field_name_表示虚拟列名(表达式/聚集/用于替代子查询表达式的名字) */
  std::string table_name_, field_name_, alias_;
};

struct FieldIdentifierHash{
  size_t operator()(const FieldIdentifier& fi) const{
    std::hash<std::string> hash_func;
    return hash_func(fi.field_name()+fi.table_name());
  }
};

// group by; 感觉groupBy不应该开一个stmt，并且resolver是在做校验，所以在这里定义了
class FieldsWithGroupBy{
public:
  FieldsWithGroupBy(const Field agg_field, const std::vector<Field> group_fields, AggregateOp op, bool with_table_name, bool visible)
    : agg_field_(agg_field), group_fields_(group_fields), op_(op), with_table_name_(with_table_name), visible_(visible){}
  Field agg_field_;
  std::vector<Field> group_fields_;
  AggregateOp op_;
  bool with_table_name_;
  bool visible_;
  std::string alias_;

  // 返回了一个FieldMeta为堆内存的Field，调用者要保证内存管理
  Field get_tmp_field() const{
    std::string field_name;
    if(with_table_name_){
      field_name = agg_str_name[op_] + "(" + agg_field_.table_name() + "." + agg_field_.field_name() + ")";
    }else{
      field_name = agg_str_name[op_] + "(" + agg_field_.field_name() + ")";
    }

    LOG_DEBUG("Field get_tmp_field()::field_name: %s", field_name.c_str());

    AttrType attr_type;
    int attr_len;
    if(AggregateOp::AGG_COUNT == op_){
      attr_type = INTS;
      attr_len = 4;
    }else if(AggregateOp::AGG_AVG == op_){
      attr_type = FLOATS;
      attr_len = 4;
    }else{
      attr_type = agg_field_.attr_type();
      attr_len = agg_field_.meta()->len();
    }
    FieldMeta* fm = new FieldMeta(field_name.c_str(), attr_type, 1, attr_len, true, false);

    // 这里返回的是nullptr，之后会不会有问题
    return Field(nullptr, fm);
  }
};