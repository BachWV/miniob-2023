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
// Created by wangyunlai.wyl on 2021/5/19.
//

#include "storage/index/bplus_tree_index.h"
#include "common/log/log.h"

BplusTreeIndex::~BplusTreeIndex() noexcept
{
  close();
}

// 
RC BplusTreeIndex::create(const char *file_name, const IndexMeta &index_meta, std::vector<const FieldMeta*> &field_metas)
{
  if (inited_) {
    LOG_WARN("Failed to create index due to the index has been created before. file_name:%s, index:%s, field:%s",
        file_name,
        index_meta.name(),
        index_meta.field());
    return RC::RECORD_OPENNED;
  }

  Index::init(index_meta, field_metas);

  int lensum=0;
  for(auto &field_meta : field_metas) {
    //types.push_back(field_meta->type());
    lensum+=field_meta->len();
  }
  RC rc = index_handler_.create(file_name,CHARS,lensum, index_meta.is_unique());
  if (RC::SUCCESS != rc) {
    LOG_WARN("Failed to create index_handler, file_name:%s, index:%s, field:%s, rc:%s",
        file_name,
        index_meta.name(),
        index_meta.field(),
        strrc(rc));
    return rc;
  }

  inited_ = true;
  LOG_INFO(
      "Successfully create index, file_name:%s, index:%s, field:%s", file_name, index_meta.name(), index_meta.field());
  return RC::SUCCESS;
}

RC BplusTreeIndex::open(const char *file_name, const IndexMeta &index_meta, std::vector<const FieldMeta*> &field_metas)
{
  if (inited_) {
    LOG_WARN("Failed to open index due to the index has been initedd before. file_name:%s, index:%s, field:%s",
        file_name,
        index_meta.name(),
        index_meta.field());
    return RC::RECORD_OPENNED;
  }

  Index::init(index_meta, field_metas);

  RC rc = index_handler_.open(file_name);
  if (RC::SUCCESS != rc) {
    LOG_WARN("Failed to open index_handler, file_name:%s, index:%s, field:%s, rc:%s",
        file_name,
        index_meta.name(),
        index_meta.field(),
        strrc(rc));
    return rc;
  }

  inited_ = true;
  LOG_INFO(
      "Successfully open index, file_name:%s, index:%s, field:%s", file_name, index_meta.name(), index_meta.field());
  return RC::SUCCESS;
}

RC BplusTreeIndex::close()
{
  if (inited_) {
    LOG_INFO("Begin to close index, index:%s, field:%s", index_meta_.name(), index_meta_.field());
    index_handler_.close();
    inited_ = false;
  }
  LOG_INFO("Successfully close index.");
  return RC::SUCCESS;
}

RC BplusTreeIndex::insert_entry(const char *record, const RID *rid)
{ 
  int len=0;
  for(auto &index_field : field_metas_){
    len+=index_field->len();
  }
  char *new_str = new char[len];
  int str_count=0;
  for(auto &index_field : field_metas_){
    //可能会获得不到index_field的name，因为初始化构造有问题
    //std::string ss=index_field->name();
    //取record的第0-4个字节，8-12字节
    //strncpy(new_str,record+index_field->offset(),index_field->len());
    strncpy(new_str+str_count,record+index_field->offset(),index_field->len());
    str_count+=index_field->len();
  }

  return index_handler_.insert_entry(new_str, rid);
}

RC BplusTreeIndex::delete_entry(const char *record, const RID *rid)
{
   int len=0;
  for(auto &index_field : field_metas_){
    len+=index_field->len();
  }
  char *new_str = new char[len];
  int str_count=0;
  for(auto &index_field : field_metas_){
    strncpy(new_str+str_count,record+index_field->offset(),index_field->len());
    str_count+=index_field->len();
  }

  return index_handler_.delete_entry(new_str, rid);
}

IndexScanner *BplusTreeIndex::create_scanner(
    const char *left_key, int left_len, bool left_inclusive, const char *right_key, int right_len, bool right_inclusive)
{
  BplusTreeIndexScanner *index_scanner = new BplusTreeIndexScanner(index_handler_);
  RC rc = index_scanner->open(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open index scanner. rc=%d:%s", rc, strrc(rc));
    delete index_scanner;
    return nullptr;
  }
  return index_scanner;
}

RC BplusTreeIndex::sync()
{
  return index_handler_.sync();
}

////////////////////////////////////////////////////////////////////////////////
BplusTreeIndexScanner::BplusTreeIndexScanner(BplusTreeHandler &tree_handler) : tree_scanner_(tree_handler)
{}

BplusTreeIndexScanner::~BplusTreeIndexScanner() noexcept
{
  tree_scanner_.close();
}

RC BplusTreeIndexScanner::open(
    const char *left_key, int left_len, bool left_inclusive, const char *right_key, int right_len, bool right_inclusive)
{
  return tree_scanner_.open(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
}

RC BplusTreeIndexScanner::next_entry(RID *rid)
{
  return tree_scanner_.next_entry(*rid);
}

RC BplusTreeIndexScanner::destroy()
{
  delete this;
  return RC::SUCCESS;
}
