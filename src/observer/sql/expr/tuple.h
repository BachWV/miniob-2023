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
// Created by Wangyunlai on 2021/5/14.
//

#pragma once

#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include "common/log/log.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/parse.h"
#include "sql/parser/value.h"
#include "sql/expr/expression.h"
#include "storage/record/record.h"

class Table;

/**
 * @defgroup Tuple
 * @brief Tuple 元组，表示一行数据，当前返回客户端时使用
 * @details 
 * tuple是一种可以嵌套的数据结构。
 * 比如select t1.a+t2.b from t1, t2;
 * 需要使用下面的结构表示：
 * @code {.cpp}
 *  Project(t1.a+t2.b)
 *        |
 *      Joined
 *      /     \
 *   Row(t1) Row(t2)
 * @endcode
 * 
 */

/**
 * @brief 元组的结构，包含哪些字段(这里成为Cell)，每个字段的说明
 * @ingroup Tuple
 */
class TupleSchema 
{
public:
  void append_cell(const TupleCellSpec &cell)
  {
    cells_.push_back(cell);
  }
  void append_cell(const char *table, const char *field)
  {
    append_cell(TupleCellSpec(table, field));
  }
  void append_cell(const char *alias)
  {
    append_cell(TupleCellSpec(alias));
  }
  int cell_num() const
  {
    return static_cast<int>(cells_.size());
  }
  const TupleCellSpec &cell_at(int i) const
  {
    return cells_[i];
  }

private:
  std::vector<TupleCellSpec> cells_;
};

/**
 * @brief 元组的抽象描述
 * @ingroup Tuple
 */
class Tuple 
{
public:
  Tuple() = default;
  virtual ~Tuple() = default;

  /**
   * @brief 获取元组中的Cell的个数
   * @details 个数应该与tuple_schema一致
   */
  virtual int cell_num() const = 0;

  /**
   * @brief 获取指定位置的Cell
   * 
   * @param index 位置
   * @param[out] cell  返回的Cell
   */
  virtual RC cell_at(int index, Value &cell) const = 0;

  /**
   * @brief 根据cell的描述，获取cell的值
   * 
   * @param spec cell的描述
   * @param[out] cell 返回的cell
   */
  virtual RC find_cell(const TupleCellSpec &spec, Value &cell) const = 0;

  // 浅拷贝生成的对象在销毁的时候会导致底层record对象被析构，其他对象无法访问record
  // 只能用这个拷贝函数
  // 由于继承和多态的限制，上层不知道底层Tuple是什么类型，该函数直接复制自己并返回基类指针
  virtual Tuple* get_replica(){return nullptr;};
  virtual bool is_data_owner(){return is_data_owner_;}

  virtual std::string to_string() const
  {
    std::string str;
    const int cell_num = this->cell_num();
    for (int i = 0; i < cell_num - 1; i++) {
      Value cell;
      cell_at(i, cell);
      str += cell.to_string();
      str += ", ";
    }

    if (cell_num > 0) {
      Value cell;
      cell_at(cell_num - 1, cell);
      str += cell.to_string();
    }
    return str;
  }
protected:
  bool is_data_owner_{false};
};

/**
 * @brief 一行数据的元组
 * @ingroup Tuple
 * @details 直接就是获取表中的一条记录
 */
class RowTuple : public Tuple 
{
public:
  RowTuple() = default;

  RowTuple(const Table *table, const std::vector<FieldExpr *>& speces){
    table_ = table;
    for(auto spece : speces){
      speces_.push_back(new FieldExpr(*spece));
    }
  }

  virtual ~RowTuple()
  {
    for (FieldExpr *spec : speces_) {
      delete spec;
    }
    speces_.clear();

    if(is_data_owner_){
      delete record_;
    }
  }

  virtual Tuple* get_replica() override{
    auto replica = new RowTuple(table_, speces_);

    replica->record_ = record_->get_replica(table_->table_meta().record_size());
    replica->is_data_owner_ = true;
    return replica;
  }

  void set_record(Record *record)
  {
    this->record_ = record;
  }

  void set_schema(const Table *table, const std::vector<FieldMeta> *fields)
  {
    table_ = table;
    this->speces_.reserve(fields->size());
    for (const FieldMeta &field : *fields) {
      speces_.push_back(new FieldExpr(table, &field));
    }
  }

  int cell_num() const override
  {
    return speces_.size();
  } 

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(speces_.size())) {
      LOG_WARN("invalid argument. index=%d", index);
      return RC::INVALID_ARGUMENT;
    }

    // 目前而言，RowTuple一定是包含表中所有字段的，这里的index就是构造表时各属性的index
    if (record_->is_null_value(index)) {
      cell.set_type(AttrType::NULL_TYPE);
      return RC::SUCCESS;
    }

    FieldExpr *field_expr = speces_[index];
    const FieldMeta *field_meta = field_expr->field().meta();
    cell.set_type(field_meta->type());
    cell.set_data(this->record_->data() + field_meta->offset(), field_meta->len());
    return RC::SUCCESS;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    const char *table_name = spec.table_name();
    const char *field_name = spec.field_name();


    // 理论上传不到这里
    assert(table_name != nullptr);
    if (0 != strcmp(table_name, table_->name())) {
      return RC::NOTFOUND;
    }

    for (size_t i = 0; i < speces_.size(); ++i) {
      const FieldExpr *field_expr = speces_[i];
      const Field &field = field_expr->field();
      if (0 == strcmp(field_name, field.field_name())) {
        return cell_at(i, cell);
      }
    }
    return RC::NOTFOUND;
  }

#if 0
  RC cell_spec_at(int index, const TupleCellSpec *&spec) const override
  {
    if (index < 0 || index >= static_cast<int>(speces_.size())) {
      LOG_WARN("invalid argument. index=%d", index);
      return RC::INVALID_ARGUMENT;
    }
    spec = speces_[index];
    return RC::SUCCESS;
  }
#endif

  Record &record()
  {
    return *record_;
  }

  const Record &record() const
  {
    return *record_;
  }

private:
  Record *record_ = nullptr;
  const Table *table_ = nullptr;
  std::vector<FieldExpr *> speces_;
};

/**
 * @brief 从一行数据中，选择部分字段组成的元组，也就是投影操作
 * @ingroup Tuple
 * @details 一般在select语句中使用。
 * 投影也可以是很复杂的操作，比如某些字段需要做类型转换、重命名、表达式运算、函数计算等。
 * 当前的实现是比较简单的，只是选择部分字段，不做任何其他操作。
 */
class ProjectTuple : public Tuple 
{
public:
  ProjectTuple() = default;
  virtual ~ProjectTuple()
  {
    for (TupleCellSpec *spec : speces_) {
      delete spec;
    }
    speces_.clear();

    if(is_data_owner_){
      delete tuple_;
    }
  }

  virtual Tuple* get_replica() override{
    auto replica = new ProjectTuple();
    for (TupleCellSpec *spec : speces_) {
      replica->speces_.push_back(new TupleCellSpec(*spec));
    }
    replica->is_data_owner_ = true;
    replica->tuple_ = tuple_->get_replica();
    return replica;
  }

  void set_tuple(Tuple *tuple)
  {
    this->tuple_ = tuple;
  }

  void add_cell_spec(TupleCellSpec *spec)
  {
    speces_.push_back(spec);
  }
  int cell_num() const override
  {
    return speces_.size();
  }

  // 我们找的时候找的是speces_中的字段，从speces拿到之后，在直接从我们绑定的普通tuple中find
  // 传入的index经过speces过滤了一遍再返回
  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(speces_.size())) {
      return RC::INTERNAL;
    }
    if (tuple_ == nullptr) {
      return RC::INTERNAL;
    }

    const TupleCellSpec *spec = speces_[index];
    return tuple_->find_cell(*spec, cell);
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    return tuple_->find_cell(spec, cell);
  }

#if 0
  RC cell_spec_at(int index, const TupleCellSpec *&spec) const override
  {
    if (index < 0 || index >= static_cast<int>(speces_.size())) {
      return RC::NOTFOUND;
    }
    spec = speces_[index];
    return RC::SUCCESS;
  }
#endif
private:
  // 已投影的vector
  std::vector<TupleCellSpec *> speces_;
  Tuple *tuple_ = nullptr;
};

class ExpressionTuple : public Tuple 
{
public:
  ExpressionTuple(std::vector<std::unique_ptr<Expression>> &expressions)
    : expressions_(expressions)
  {
  }
  
  virtual ~ExpressionTuple()
  {
  }

  int cell_num() const override
  {
    return expressions_.size();
  }

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= static_cast<int>(expressions_.size())) {
      return RC::INTERNAL;
    }

    const Expression *expr = expressions_[index].get();
    return expr->try_get_value(cell);
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    for (const std::unique_ptr<Expression> &expr : expressions_) {
      if (0 == strcmp(spec.alias(), expr->name().c_str())) {
        return expr->try_get_value(cell);
      }
    }
    return RC::NOTFOUND;
  }


private:
  const std::vector<std::unique_ptr<Expression>> &expressions_;
};

/**
 * @brief 一些常量值组成的Tuple
 * @ingroup Tuple
 */
class ValueListTuple : public Tuple 
{
public:
  ValueListTuple() = default;
  virtual ~ValueListTuple() = default;

  void set_cells(const std::vector<Value> &cells)
  {
    cells_ = cells;
  }

  virtual int cell_num() const override
  {
    return static_cast<int>(cells_.size());
  }

  virtual RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= cell_num()) {
      return RC::NOTFOUND;
    }

    cell = cells_[index];
    return RC::SUCCESS;
  }

  virtual RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    return RC::INTERNAL;
  }

private:
  std::vector<Value> cells_;
};

/**
 * @brief 将两个tuple合并为一个tuple
 * @ingroup Tuple
 * @details 在join算子中使用
 */
class JoinedTuple : public Tuple 
{
public:
  JoinedTuple() = default;
  virtual ~JoinedTuple(){
    if(is_data_owner_){
      delete left_;
      delete right_;
    }
  }

  virtual Tuple* get_replica() override{
    auto replica = new JoinedTuple();
    replica->left_ = left_->get_replica();
    replica->right_ = right_->get_replica();
    replica->is_data_owner_ = true;
    return replica;
  }

  void set_left(Tuple *left)
  {
    left_ = left;
  }
  void set_right(Tuple *right)
  {
    right_ = right;
  }

  int cell_num() const override
  {
    return left_->cell_num() + right_->cell_num();
  }

  RC cell_at(int index, Value &value) const override
  {
    const int left_cell_num = left_->cell_num();
    if (index > 0 && index < left_cell_num) {
      return left_->cell_at(index, value);
    }

    if (index >= left_cell_num && index < left_cell_num + right_->cell_num()) {
      return right_->cell_at(index - left_cell_num, value);
    }

    return RC::NOTFOUND;
  }

  RC find_cell(const TupleCellSpec &spec, Value &value) const override
  {
    RC rc = left_->find_cell(spec, value);
    if (rc == RC::SUCCESS || rc != RC::NOTFOUND) {
      return rc;
    }

    return right_->find_cell(spec, value);
  }

private:
  Tuple *left_ = nullptr;
  Tuple *right_ = nullptr;
};


class AddOneFieldTuple: public Tuple{
public:
  AddOneFieldTuple() = default;
  AddOneFieldTuple(Tuple* original, FieldMeta new_field_meta): original_(original), new_field_meta_(new_field_meta){
    new_value_.set_type(new_field_meta.type());
  }
  virtual ~AddOneFieldTuple(){
    if(is_data_owner_){
      delete original_;
      // new_value是一个栈上的字段，自己会析构，不需要delete
    }
  }

  virtual int cell_num() const override{
    if(original_){
      return original_->cell_num() +1;
    }else{
      // function最底层tuple
      return 1;
    }
  }

  virtual RC cell_at(int index, Value &cell) const override{

    if(original_){
      const int original_num = original_->cell_num();
      if(index >= 0 && index < original_num){
        return original_->cell_at(index, cell);
      }else if( index == original_num){
        cell = new_value_;
        return RC::SUCCESS;
      }
      return RC::NOTFOUND;
    }else{
      // Function里面无table的最底层aof field,cell at不能先判断original的数目
      // aof_tuple1: null       + value1  // target
      // aof_tuple2: aof_tuple1 + value2
      // aof_tuple3: aof_tuple2 + value3
      assert(index == 0);
      cell = new_value_;
      return RC::SUCCESS;
    }
  }

  virtual RC find_cell(const TupleCellSpec &spec, Value &cell) const override{
    // 虚拟字段没写表名的判断
    if(0 == strcasecmp( spec.field_name(), new_field_meta_.name())){
      cell = new_value_;
      return RC::SUCCESS;
    }

    // 如果find的不是虚拟字段，那么original必须有tuple，就算是function最底层aoftuple也应该接住字段的查找
    assert(original_);
    RC rc = original_->find_cell(spec, cell);
    if (rc == RC::SUCCESS || rc != RC::NOTFOUND) {
      return rc;
    }
    
    return RC::NOTFOUND;
  }

  // Value中的数据自己会复制
  virtual Tuple* get_replica() override{
    auto replica = new AddOneFieldTuple(original_->get_replica(), new_field_meta_);
    replica->set_new_field_value(new_value_);
    replica->is_data_owner_ = true;
    return replica;
  };

  void set_original(Tuple *original){
    original_ = original;
  }

  void set_new_field_meta(FieldMeta new_field_meta){
    new_field_meta_ = new_field_meta;
  }

  // 用于设置值
  void set_new_field_data(char *data, int length){
    new_value_.set_data(data, length);
  }

  // set_value底层会拿到引用数据的值，以值拷贝的形式复制一份
  void set_new_field_value(const Value& v){
    new_value_.set_value(v);
  }

  // value里面一堆方法，直接把引用返回给上层，上层想干啥就干啥吧
  Value& get_new_field_value(){
    return new_value_;
  }

private:
  Tuple *original_{nullptr};
  FieldMeta new_field_meta_;
  Value new_value_;
};