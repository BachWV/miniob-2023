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
// Created by Wangyunlai on 2022/5/4.
//

#pragma once

#include <stddef.h>
#include <vector>
#include <limits>
#include <sstream>

#include "common/rc.h"
#include "common/types.h"
#include "common/log/log.h"
#include "storage/index/index_meta.h"
#include "storage/field/field_meta.h"

class Field;

/**
 * @brief 标识一个记录的位置
 * 一个记录是放在某个文件的某个页面的某个槽位。这里不记录文件信息，记录页面和槽位信息
 */
struct RID
{
  PageNum page_num;  // record's page number
  SlotNum slot_num;  // record's slot number

  RID() = default;
  RID(const PageNum _page_num, const SlotNum _slot_num) : page_num(_page_num), slot_num(_slot_num) {}

  const std::string to_string() const
  {
    std::stringstream ss;
    ss << "PageNum:" << page_num << ", SlotNum:" << slot_num;
    return ss.str();
  }

  bool operator==(const RID &other) const { return page_num == other.page_num && slot_num == other.slot_num; }

  bool operator!=(const RID &other) const { return !(*this == other); }

  static int compare(const RID *rid1, const RID *rid2)
  {
    int page_diff = rid1->page_num - rid2->page_num;
    if (page_diff != 0) {
      return page_diff;
    } else {
      return rid1->slot_num - rid2->slot_num;
    }
  }

  /**
   * 返回一个不可能出现的最小的RID
   * 虽然page num 0和slot num 0都是合法的，但是page num 0通常用于存放meta数据，所以对数据部分来说都是
   * 不合法的. 这里在bplus tree中查找时会用到。
   */
  static RID *min()
  {
    static RID rid{0, 0};
    return &rid;
  }

  /**
   * @brief 返回一个“最大的”RID
   * 我们假设page num和slot num都不会使用对应数值类型的最大值
   */
  static RID *max()
  {
    static RID rid{std::numeric_limits<PageNum>::max(), std::numeric_limits<SlotNum>::max()};
    return &rid;
  }
};

/**
 * @brief 表示一个记录
 * 当前的记录都是连续存放的空间（内存或磁盘上）。
 * 为了提高访问的效率，record通常直接记录指向页面上的内存，但是需要保证访问这种数据时，拿着锁资源。
 * 为了方便，也提供了复制内存的方法。可以参考set_data_owner
 */
class Record
{
public:
  Record() = default;
  ~Record()
  {
    if (owner_ && data_ != nullptr) {
      free(data_);
      data_ = nullptr;
    }
  }

  Record(const Record &other)
  {
    rid_   = other.rid_;
    data_  = other.data_;
    len_   = other.len_;
    owner_ = other.owner_;
    is_null = other.is_null;

    if (other.owner_) {
      char *tmp = (char *)malloc(other.len_);
      ASSERT(nullptr != tmp, "failed to allocate memory. size=%d", other.len_);
      memcpy(tmp, other.data_, other.len_);
      data_ = tmp;
    }
  }

  Record &operator=(const Record &other)
  {
    if (this == &other) {
      return *this;
    }

    this->~Record();
    new (this) Record(other);
    return *this;
  }

  Record* get_replica(int record_size = 0){
    auto replica = new Record();
    replica->rid_   = rid_;
    replica->len_   = record_size == 0 ? len_ : record_size;
    replica->owner_ = true;
    replica->is_null = is_null;

    char *tmp = (char *)malloc(replica->len_);
    ASSERT(nullptr != tmp, "failed to allocate memory. size=%d", replica->len_);
    memcpy(tmp, data_, replica->len_);
    replica->data_ = tmp;

    return replica;
  }

  void set_data(char *data, int len = 0)
  {
    this->data_ = data;
    this->len_  = len;
  }
  void set_data_owner(char *data, int len)
  {
    ASSERT(len != 0, "the len of data should not be 0");
    this->~Record();

    this->data_  = data;
    this->len_   = len;
    this->owner_ = true;
  }

  char       *data() { return this->data_; }
  const char *data() const { return this->data_; }
  int         len() const { return this->len_; }

  void set_rid(const RID &rid) { this->rid_ = rid; }
  void set_rid(const PageNum page_num, const SlotNum slot_num)
  {
    this->rid_.page_num = page_num;
    this->rid_.slot_num = slot_num;
  }
  RID       &rid() { return rid_; }
  const RID &rid() const { return rid_; }

  /* index从0开始，判断第index个属性是否为空值 */
  bool is_null_value(int index) const {
    if(index >= is_null.size())
      return false;
    return is_null[index];
  }

  void set_null_flag(int index) {
    if (index >= is_null.size())
      is_null.resize(index + 1, false);
    is_null[index] = true;
  }

  const std::vector<bool> &get_null_states() const {
    return is_null;
  }

  void set_null_states(const std::vector<bool> &null_states) {
    is_null = null_states;
  }

  void clear_null_states() {
    is_null.clear();
  }

  void set_null_states_size(int bit_num) {
    is_null.resize(bit_num, false);
  }

private:
  RID rid_;

  char *data_  = nullptr;
  int   len_   = 0;       /// 如果不是record自己来管理内存，这个字段可能是无效的
  bool  owner_ = false;   /// 表示当前是否由record来管理内存

  // 空值位图。这个位图是给读记录时的读者的，因为读者无法从data_中提取位图。插入新记录时，位图通过table->make_record直接记在data_中。
  std::vector<bool> is_null;
};
