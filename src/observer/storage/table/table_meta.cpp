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
// Created by Meiyi & Wangyunlai on 2021/5/12.
//

#include <algorithm>
#include <common/lang/string.h>

#include "storage/table/table_meta.h"
#include "json/json.h"
#include "common/log/log.h"
#include "storage/trx/trx.h"

using namespace std;

static const Json::StaticString FIELD_TABLE_ID("table_id");
static const Json::StaticString FIELD_TABLE_NAME("table_name");
static const Json::StaticString FIELD_FIELDS("fields");
static const Json::StaticString FIELD_INDEXES("indexes");
static const Json::StaticString FIELD_RECORD_SIZE("record_size");
static const Json::StaticString FIELD_NULL_BITMAP_OFFSET("null_bitmap_offset");
static const Json::StaticString FIELD_NULL_BITMAP_SIZE("null_bitmap_size");

TableMeta::TableMeta(const TableMeta &other)
    : table_id_(other.table_id_),
    name_(other.name_),
    fields_(other.fields_),
    indexes_(other.indexes_),
    record_size_(other.record_size_),
    null_bitmap_offset(other.null_bitmap_offset),
    null_bitmap_len(other.null_bitmap_len)
{}

void TableMeta::swap(TableMeta &other) noexcept
{
  name_.swap(other.name_);
  fields_.swap(other.fields_);
  indexes_.swap(other.indexes_);
  std::swap(record_size_, other.record_size_);
}

// 用给定的attr初始表的meta
RC TableMeta::init(int32_t table_id, const char *name, int field_num, const AttrInfoSqlNode attributes[])
{
  if (common::is_blank(name)) {
    LOG_ERROR("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }

  if (field_num <= 0 || nullptr == attributes) {
    LOG_ERROR("Invalid argument. name=%s, field_num=%d, attributes=%p", name, field_num, attributes);
    return RC::INVALID_ARGUMENT;
  }

  RC rc = RC::SUCCESS;
  
  int field_offset = 0;
  int trx_field_num = 0;
  const vector<FieldMeta> *trx_fields = TrxKit::instance()->trx_fields();
  if (trx_fields != nullptr) {
    fields_.resize(field_num + trx_fields->size());

    for (size_t i = 0; i < trx_fields->size(); i++) {
      const FieldMeta &field_meta = (*trx_fields)[i];
      fields_[i] = FieldMeta(field_meta.name(), field_meta.type(), field_offset, field_meta.len(), false/*visible*/);
      field_offset += field_meta.len();
    }

    trx_field_num = static_cast<int>(trx_fields->size());
  } else {
    fields_.resize(field_num);
  }

  for (int i = 0; i < field_num; i++) {
    const AttrInfoSqlNode &attr_info = attributes[i];
    rc = fields_[i + trx_field_num].init(attr_info.name.c_str(), 
            attr_info.type, field_offset, attr_info.length, true/*visible*/, attr_info.nullable);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to init field meta. table name=%s, field name: %s", name, attr_info.name.c_str());
      return rc;
    }

    field_offset += attr_info.length;
  }

  /* 增加对nullable的支持，在末尾增加一个隐藏属性，表示空位图
   * 但该隐藏属性不能放在fields_数组中，否则和其它部分不兼容(desc table, select *, ...)，需要修改的更多
   * 此处选择使用额外的数据结构(null_bitmap_offset和null_bitmap_len)记录null_bitmap的信息
   * 注意此处位图也包括sys_fields，这样可以在各处统一访问属性的下标
   * 
   * 由于表元数据在序列化为文件时，没有写入record_size，它是在反序列化时根据各个field长度计算出来的
   * 这里不把null_bitmap放在field中，就需要把record_size，描述位图的数据都进行序列化
   */
  null_bitmap_offset = field_offset;
  null_bitmap_len = (field_num + trx_field_num + 7) / 8;  // 向上取整，位图需要的字节数

  record_size_ = field_offset + null_bitmap_len;

  table_id_ = table_id;
  name_     = name;
  LOG_INFO("Sussessfully initialized table meta. table id=%d, name=%s", table_id, name);
  return RC::SUCCESS;
}

RC TableMeta::add_index(const IndexMeta &index)
{
  indexes_.push_back(index);
  return RC::SUCCESS;
}

const char *TableMeta::name() const
{
  return name_.c_str();
}

const FieldMeta *TableMeta::trx_field() const
{
  return &fields_[0];
}

const std::pair<const FieldMeta *, int> TableMeta::trx_fields() const
{
  return std::pair<const FieldMeta *, int>{fields_.data(), sys_field_num()};
}

const FieldMeta *TableMeta::field(int index) const
{
  return &fields_[index];
}
const FieldMeta *TableMeta::field(const char *name) const
{
  if (nullptr == name) {
    return nullptr;
  }
  for (const FieldMeta &field : fields_) {
    if (0 == strcmp(field.name(), name)) {
      return &field;
    }
  }
  return nullptr;
}

const FieldMeta *TableMeta::find_field_by_offset(int offset) const
{
  for (const FieldMeta &field : fields_) {
    if (field.offset() == offset) {
      return &field;
    }
  }
  return nullptr;
}
int TableMeta::field_num() const
{
  return fields_.size();
}

int TableMeta::sys_field_num() const
{
  const vector<FieldMeta> *trx_fields = TrxKit::instance()->trx_fields();
  if (nullptr == trx_fields) {
    return 0;
  }
  return static_cast<int>(trx_fields->size());
}

const IndexMeta *TableMeta::index(const char *name) const
{
  for (const IndexMeta &index : indexes_) {
    if (0 == strcmp(index.name(), name)) {
      return &index;
    }
  }
  return nullptr;
}

const IndexMeta *TableMeta::find_index_by_field(const char *field) const
{
  for (const IndexMeta &index : indexes_) {
    if (0 == strcmp(index.field(), field)) {
      return &index;
    }
  }
  return nullptr;
}

const IndexMeta *TableMeta::index(int i) const
{
  return &indexes_[i];
}

int TableMeta::index_num() const
{
  return indexes_.size();
}

int TableMeta::record_size() const
{
  return record_size_;
}

int TableMeta::serialize(std::ostream &ss) const
{

  Json::Value table_value;
  table_value[FIELD_TABLE_ID]   = table_id_;
  table_value[FIELD_TABLE_NAME] = name_;

  Json::Value fields_value;
  for (const FieldMeta &field : fields_) {
    Json::Value field_value;
    field.to_json(field_value);
    fields_value.append(std::move(field_value));
  }

  table_value[FIELD_FIELDS] = std::move(fields_value);

  Json::Value indexes_value;
  for (const auto &index : indexes_) {
    Json::Value index_value;
    index.to_json(index_value);
    indexes_value.append(std::move(index_value));
  }
  table_value[FIELD_INDEXES] = std::move(indexes_value);

  /* 增加record_size, null_bitmap信息的序列化 */
  table_value[FIELD_RECORD_SIZE] = record_size_;
  table_value[FIELD_NULL_BITMAP_OFFSET] = null_bitmap_offset;
  table_value[FIELD_NULL_BITMAP_SIZE] = null_bitmap_len;
 
  Json::StreamWriterBuilder builder;
  Json::StreamWriter *writer = builder.newStreamWriter();

  std::streampos old_pos = ss.tellp();
  writer->write(table_value, &ss);
  int ret = (int)(ss.tellp() - old_pos);

  delete writer;
  return ret;
}

int TableMeta::deserialize(std::istream &is)
{
  Json::Value table_value;
  Json::CharReaderBuilder builder;
  std::string errors;

  std::streampos old_pos = is.tellg();
  if (!Json::parseFromStream(builder, is, &table_value, &errors)) {
    LOG_ERROR("Failed to deserialize table meta. error=%s", errors.c_str());
    return -1;
  }

  const Json::Value &table_id_value = table_value[FIELD_TABLE_ID];
  if (!table_id_value.isInt()) {
    LOG_ERROR("Invalid table id. json value=%s", table_id_value.toStyledString().c_str());
    return -1;
  }

  int32_t table_id = table_id_value.asInt();

  const Json::Value &table_name_value = table_value[FIELD_TABLE_NAME];
  if (!table_name_value.isString()) {
    LOG_ERROR("Invalid table name. json value=%s", table_name_value.toStyledString().c_str());
    return -1;
  }

  std::string table_name = table_name_value.asString();

  const Json::Value &fields_value = table_value[FIELD_FIELDS];
  if (!fields_value.isArray() || fields_value.size() <= 0) {
    LOG_ERROR("Invalid table meta. fields is not array, json value=%s", fields_value.toStyledString().c_str());
    return -1;
  }

  RC rc = RC::SUCCESS;
  int field_num = fields_value.size();
  std::vector<FieldMeta> fields(field_num);
  for (int i = 0; i < field_num; i++) {
    FieldMeta &field = fields[i];

    const Json::Value &field_value = fields_value[i];
    rc = FieldMeta::from_json(field_value, field);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to deserialize table meta. table name =%s", table_name.c_str());
      return -1;
    }
  }

  auto comparator = [](const FieldMeta &f1, const FieldMeta &f2) { return f1.offset() < f2.offset(); };
  std::sort(fields.begin(), fields.end(), comparator);

  table_id_ = table_id;
  name_.swap(table_name);
  fields_.swap(fields);
  // record_size_ = fields_.back().offset() + fields_.back().len() - fields_.begin()->offset();

  /* 增加NULL支持：record_size不再根据fields计算出来，而是直接反序列化 */
  const Json::Value &record_size_value = table_value[FIELD_RECORD_SIZE];
  if (!record_size_value.isInt()) {
    LOG_ERROR("Invalid record size. json value=%s", record_size_value.toStyledString().c_str());
    return -1;
  }
  record_size_ = record_size_value.asInt();

  const Json::Value &null_bitmap_offset_value = table_value[FIELD_NULL_BITMAP_OFFSET];
  if (!null_bitmap_offset_value.isInt()) {
    LOG_ERROR("Invalid null bitmap offset. json value=%s", null_bitmap_offset_value.toStyledString().c_str());
    return -1;
  }
  null_bitmap_offset = null_bitmap_offset_value.asInt();

  const Json::Value &null_bitmap_len_value = table_value[FIELD_NULL_BITMAP_SIZE];
  if (!null_bitmap_len_value.isInt()) {
    LOG_ERROR("Invalid null bitmap offset. json value=%s", null_bitmap_len_value.toStyledString().c_str());
    return -1;
  }
  null_bitmap_len = null_bitmap_len_value.asInt();

  const Json::Value &indexes_value = table_value[FIELD_INDEXES];
  if (!indexes_value.empty()) {
    if (!indexes_value.isArray()) {
      LOG_ERROR("Invalid table meta. indexes is not array, json value=%s", fields_value.toStyledString().c_str());
      return -1;
    }
    const int index_num = indexes_value.size();
    std::vector<IndexMeta> indexes(index_num);
    for (int i = 0; i < index_num; i++) {
      IndexMeta &index = indexes[i];

      const Json::Value &index_value = indexes_value[i];
      rc = IndexMeta::from_json(*this, index_value, index);
      if (rc != RC::SUCCESS) {
        LOG_ERROR("Failed to deserialize table meta. table name=%s", table_name.c_str());
        return -1;
      }
    }
    indexes_.swap(indexes);
  }

  return (int)(is.tellg() - old_pos);
}

int TableMeta::get_serial_size() const
{
  return -1;
}

void TableMeta::to_string(std::string &output) const
{}

void TableMeta::desc(std::ostream &os) const
{
  os << name_ << '(' << std::endl;
  for (const auto &field : fields_) {
    os << '\t';
    field.desc(os);
    os << std::endl;
  }

  for (const auto &index : indexes_) {
    os << '\t';
    index.desc(os);
    os << std::endl;
  }
  os << ')' << std::endl;
}


bool TableMeta::has_unique_index() const
{
  for (const auto &index : indexes_) {
    if (index.is_unique()) {
      return true;
    }
  }
  return false;
}

void TableMeta::find_unique_index_field_vector(std::vector<std::vector<int>> &field_need_to_cmp_list_list) const
{
  for(const auto &index : indexes_){
    if(index.is_unique()==false){
      continue;//对于非unique的索引，不需要去重
    }
    //第i个索引，该索引可以包括多个col
    std::vector<int> field_need_to_cmp_list;
    std::vector<std::string> index_field_name_list = index.fields();//在哪几个列建立了索引
    assert(index_field_name_list.size()>0);//理论上必须得有index的列名
    for(int i = 0 ; i < field_num() ; i++){
      auto field_name = fields_[i].name();
      auto it = std::find_if(index_field_name_list.begin(),index_field_name_list.end(),[&field_name](const std::string &s){return strcmp(s.c_str(),field_name)==0;});//O(nm)的列名匹配

      if(it != index_field_name_list.end()){
        field_need_to_cmp_list.push_back(i);
      }
    }
    //理论上必须找到，否则说明索引的列名有问题
    assert(field_need_to_cmp_list.size()>0);
    field_need_to_cmp_list_list.push_back(field_need_to_cmp_list);
  }
}