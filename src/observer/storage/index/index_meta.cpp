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
// Created by Wangyunlai.wyl on 2021/5/18.
//

#include "storage/index/index_meta.h"
#include "storage/field/field_meta.h"
#include "storage/table/table_meta.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "json/json.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");

RC IndexMeta::init(const char *name, std::vector<const FieldMeta *> field_metas,bool is_unique)
{
  if (common::is_blank(name)) {
    LOG_ERROR("Failed to init index, name is empty.");
    return RC::INVALID_ARGUMENT;
  }

  name_ = name;
  is_unique_ = is_unique;
  for(auto &field_meta : field_metas){
    fields_.push_back(field_meta->name());
  }
  return RC::SUCCESS;
}

void IndexMeta::to_json(Json::Value &json_value) const
{
  json_value[FIELD_NAME] = name_;
  //for(auto &field : fields_)
  Json::Value field_name_list;
  for(auto &field : fields_)
    field_name_list.append(field);
  json_value[FIELD_FIELD_NAME] = field_name_list;
  json_value["is_unique"] = is_unique_;
}

RC IndexMeta::from_json(const TableMeta &table, const Json::Value &json_value, IndexMeta &index)
{
  const Json::Value &name_value = json_value[FIELD_NAME];
  const Json::Value &field_name_list = json_value[FIELD_FIELD_NAME];
  const Json::Value &is_unique_value = json_value["is_unique"];
  if (!name_value.isString()) {
    LOG_ERROR("Index name is not a string. json value=%s", name_value.toStyledString().c_str());
    return RC::INTERNAL;
  }

  if (!field_name_list.isArray() || field_name_list.size() <=0) {
    LOG_ERROR("Index field is not an array or empty. json value=%s", field_name_list.toStyledString().c_str());
    return RC::INTERNAL;
  }

  int field_num = field_name_list.size();
  std::vector<const FieldMeta *> field_meta_list;
  for(int i=0;i<field_num;i++){
    const Json::Value &field_value = field_name_list[i];
    if (!field_value.isString()) {
      LOG_ERROR("Index field is not a string. json value=%s", field_value.toStyledString().c_str());
      return RC::INTERNAL;
    }
    const FieldMeta *field = table.field(field_value.asCString());
    if (nullptr == field) {
      LOG_ERROR("Deserialize index [%s]: no such field: %s", name_value.asCString(), field_value.asCString());
      return RC::SCHEMA_FIELD_MISSING;
    }
    field_meta_list.push_back(field);
  }
  if(!is_unique_value.isBool()){
    LOG_ERROR("Index is_unique is not a bool. json value=%s", is_unique_value.toStyledString().c_str());
    return RC::INTERNAL;
  }




  return index.init(name_value.asCString(), field_meta_list,is_unique_value.asBool());
}

const char *IndexMeta::name() const
{
  return name_.c_str();
}

const char *IndexMeta::field() const
{
  return fields_[0].c_str();
}

void IndexMeta::desc(std::ostream &os) const
{
  os << "index name=" << name_ << ", field=" << field_;
}