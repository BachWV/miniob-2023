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

#include <string.h>
#include <memory>
#include <string>

#include "storage/field/field.h"
#include "sql/parser/value.h"
#include "common/log/log.h"
#include "sql/expr/expr_attr.h"

class PhysicalOperator;
class Tuple;
class FunctionKernel;

/**
 * @defgroup Expression
 * @brief 表达式
 */

/**
 * @brief 表达式类型
 * @ingroup Expression
 */
enum class ExprType 
{
  NONE,
  STAR,         ///< 星号，表示所有字段
  FIELD,        ///< 字段。在实际执行时，根据行数据内容提取对应字段的值
  VALUE,        ///< 常量值
  CAST,         ///< 需要做类型转换的表达式
  COMPARISON,   ///< 需要做比较的表达式
  CONJUNCTION,  ///< 多个表达式使用同一种关系(AND或OR)来联结
  ARITHMETIC,   ///< 算术运算
  CORRELATE,    // 子查询关联表达式
  IDENTIFIER,   // 我们自己定义的字段表达式，可用于表列名和虚拟列名
  PREDNULL,     // is null/is not null谓词
  SELECTEXPR,
  COMPNULL,
  LIKE,         // like谓词
  QUANTIFIED_COMP_EXPR_SET,  // 集合比较: expr in/not in/all/any... (expr, expr, ...)
  FUNCTION,
};

/**
 * @brief 表达式的抽象描述
 * @ingroup Expression
 * @details 在SQL的元素中，任何需要得出值的元素都可以使用表达式来描述
 * 比如获取某个字段的值、比较运算、类型转换
 * 当然还有一些当前没有实现的表达式，比如算术运算。
 *
 * 通常表达式的值，是在真实的算子运算过程中，拿到具体的tuple后
 * 才能计算出来真实的值。但是有些表达式可能就表示某一个固定的
 * 值，比如ValueExpr。
 */
class Expression 
{
public:
  Expression() = default;
  virtual ~Expression() = default;

  /**
   * @brief 根据具体的tuple，来计算当前表达式的值。tuple有可能是一个具体某个表的行数据
   */
  virtual RC get_value(const Tuple &tuple, Value &value) const = 0;

  /**
   * @brief 在没有实际运行的情况下，也就是无法获取tuple的情况下，尝试获取表达式的值
   * @details 有些表达式的值是固定的，比如ValueExpr，这种情况下可以直接获取值
   */
  virtual RC try_get_value(Value &value) const
  {
    return RC::UNIMPLENMENT;
  }

  /**
   * @brief 表达式的类型
   * 可以根据表达式类型来转换为具体的子类
   */
  virtual ExprType type() const = 0;

  /**
   * @brief 表达式值的类型
   * @details 一个表达式运算出结果后，只有一个值
   */
  virtual AttrType value_type() const = 0;

  /*
   * 表达式的完整值属性，弃用value_type()，但保持兼容性，不删
   */
  virtual ExprValueAttr value_attr() const = 0;

  /*
   * 这个表达式中，引用了哪些表里的数据。这里的表可以是视图
   */
  virtual std::vector<std::string> referred_tables() const = 0;

  /**
   * @brief 表达式的名字，比如是字段名称，或者用户在执行SQL语句时输入的内容
   */
  virtual std::string name() const { return name_; }
  virtual void set_name(std::string name) { name_ = name; }

private:
  std::string  name_;
};

/**
 * @brief 字段表达式
 * @ingroup Expression
 */
class FieldExpr : public Expression 
{
public:
  FieldExpr() = default;
  FieldExpr(const Table *table, const FieldMeta *field) : field_(table, field)
  {}
  FieldExpr(const Field &field) : field_(field)
  {}

  virtual ~FieldExpr() = default;

  ExprType type() const override { return ExprType::FIELD; }
  AttrType value_type() const override { return field_.attr_type(); }

  /* DON'T USE!! */
  ExprValueAttr value_attr() const override { return ExprValueAttr(); }

  Field &field() { return field_; }

  const Field &field() const { return field_; }

  const char *table_name() const { return field_.table_name(); }

  const char *field_name() const { return field_.field_name(); }

  RC get_value(const Tuple &tuple, Value &value) const override;

  std::vector<std::string> referred_tables() const override { return {field_.table_name()}; }

private:
  Field field_;
};

/**
 * @brief 常量值表达式
 * @ingroup Expression
 */
class ValueExpr : public Expression 
{
public:
  ValueExpr() = default;
  explicit ValueExpr(const Value &value) : value_(value)
  {}

  virtual ~ValueExpr() = default;

  RC get_value(const Tuple &tuple, Value &value) const override;
  RC try_get_value(Value &value) const override { value = value_; return RC::SUCCESS; }

  ExprType type() const override { return ExprType::VALUE; }

  AttrType value_type() const override { return value_.attr_type(); }

  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = value_.attr_type();
    attr.length = value_.length();
    attr.nullable = true;
    return attr;
  }

  void get_value(Value &value) const { value = value_; }

  const Value &get_value() const { return value_; }

  std::vector<std::string> referred_tables() const override { return {}; }

private:
  Value value_;
};

/**
 * @brief 类型转换表达式
 * @ingroup Expression
 */
class CastExpr : public Expression 
{
public:
  CastExpr(std::unique_ptr<Expression> child, AttrType cast_type);
  virtual ~CastExpr();

  ExprType type() const override
  {
    return ExprType::CAST;
  }
  RC get_value(const Tuple &tuple, Value &value) const override;

  RC try_get_value(Value &value) const override;

  AttrType value_type() const override { return cast_type_; }

  /* DON'T USE!!! */
  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = cast_type_;
    attr.length = 4;
    attr.nullable = true;
    return attr;
  }

  std::unique_ptr<Expression> &child() { return child_; }

  std::vector<std::string> referred_tables() const override { return child_->referred_tables(); }

private:
  RC cast(const Value &value, Value &cast_value) const;

private:
  std::unique_ptr<Expression> child_;  ///< 从这个表达式转换
  AttrType cast_type_;  ///< 想要转换成这个类型
};

/**
 * @brief 比较表达式
 * @ingroup Expression
 */
class ComparisonExpr : public Expression 
{
public:
  ComparisonExpr(CompOp comp, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
  virtual ~ComparisonExpr();

  ExprType type() const override { return ExprType::COMPARISON; }

  RC get_value(const Tuple &tuple, Value &value) const override;

  AttrType value_type() const override { return BOOLEANS; }

  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = BOOLEANS;
    attr.length = 1;
    attr.nullable = false;
    return attr;
  }

  CompOp comp() const { return comp_; }

  std::unique_ptr<Expression> &left()  { return left_;  }
  std::unique_ptr<Expression> &right() { return right_; }

  /**
   * 尝试在没有tuple的情况下获取当前表达式的值
   * 在优化的时候，可能会使用到
   */
  RC try_get_value(Value &value) const override;

  /**
   * compare the two tuple cells
   * @param value the result of comparison
   */
  RC compare_value(const Value &left, const Value &right, bool &value) const;

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  CompOp comp_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
  std::vector<std::string> referred_tables_;
};

/**
 * @brief 联结表达式
 * @ingroup Expression
 * 多个表达式使用同一种关系(AND或OR)来联结
 * 当前miniob仅有AND操作
 */
class ConjunctionExpr : public Expression 
{
public:
  enum class Type {
    AND,
    OR,
  };

public:
  ConjunctionExpr(Type type, std::vector<std::unique_ptr<Expression>> &children);
  virtual ~ConjunctionExpr() = default;

  ExprType type() const override { return ExprType::CONJUNCTION; }

  AttrType value_type() const override { return BOOLEANS; }

  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = BOOLEANS;
    attr.length = 1;
    attr.nullable = false;
    return attr;
  }

  RC get_value(const Tuple &tuple, Value &value) const override;

  Type conjunction_type() const { return conjunction_type_; }

  std::vector<std::unique_ptr<Expression>> &children() { return children_; }

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  Type conjunction_type_;
  std::vector<std::unique_ptr<Expression>> children_;
  std::vector<std::string> referred_tables_;
};

/**
 * @brief 算术表达式
 * @ingroup Expression
 */
class ArithmeticExpr : public Expression 
{
public:
  enum class Type {
    ADD,
    SUB,
    MUL,
    DIV,
    NEGATIVE,
  };

public:
  ArithmeticExpr(Type type, Expression *left, Expression *right);
  ArithmeticExpr(Type type, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
  virtual ~ArithmeticExpr() = default;

  ExprType type() const override { return ExprType::ARITHMETIC; }

  AttrType value_type() const override;

  ExprValueAttr value_attr() const override;

  RC get_value(const Tuple &tuple, Value &value) const override;
  RC try_get_value(Value &value) const override;

  Type arithmetic_type() const { return arithmetic_type_; }

  std::unique_ptr<Expression> &left() { return left_; }
  std::unique_ptr<Expression> &right() { return right_; }

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  RC calc_value(const Value &left_value, const Value &right_value, Value &value) const;
  
private:
  Type arithmetic_type_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
  std::vector<std::string> referred_tables_;
};

/*
 * 用于相关子查询
 * 当前层的查询维护它的子查询中，所有引用当前层Field的表达式
 * 此表达式的值由当前层在调用子查询前设置，子查询中使用get_value获得值时，直接获得当前层设置的值
 */
class CorrelateExpr: public Expression
{
public:
  CorrelateExpr(const FieldIdentifier &correlate_identifier) : correlate_field_(correlate_identifier) {}

  // 直接返回设置好的value_
  RC get_value(const Tuple &tuple, Value &value) const override;

  ExprType type() const override { return ExprType::CORRELATE; }

  AttrType value_type() const override{ return value_attr_.type; }

  ExprValueAttr value_attr() const override { return value_attr_; }

  // 外层子查询在调用内层子查询之前，设置内层子查询里的相关表达式值，tuple是外层子查询的当前行。
  RC set_value_from_tuple(const Tuple &tuple);

  std::string identifier_name() const { return correlate_field_.table_name() + correlate_field_.field_name(); }

  void set_value_attr(const ExprValueAttr &attr) { value_attr_ = attr; }

  void set_referred_table(const std::string &table) { referred_tables_.emplace_back(table); }

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  FieldIdentifier correlate_field_;
  Value value_;
  ExprValueAttr value_attr_;
  std::vector<std::string> referred_tables_;
};

/* 字段名表达式。FieldExpr和Field对象耦合，涉及到别名、虚拟列名时不好用，这里额外写一个 */
class IdentifierExpr: public Expression
{
public:
  IdentifierExpr(const FieldIdentifier &field) : field_(field) {}

  RC get_value(const Tuple &tuple, Value &value) const override;

  ExprType type() const override{ return ExprType::IDENTIFIER; }

  /* 在FieldIdentifier内维护类型代价高，直接通过get_value返回的value就能拿到类型了，不应该用这个接口 */
  AttrType value_type() const override{ return value_attr_.type; }

  ExprValueAttr value_attr() const override { return value_attr_; }

  const FieldIdentifier &field() const { return field_; }

  void set_value_attr(const ExprValueAttr &attr) { value_attr_ = attr; }

  void set_referred_table(const std::string &table) { referred_tables_.emplace_back(table); }

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  FieldIdentifier field_;
  ExprValueAttr value_attr_;
  std::vector<std::string> referred_tables_;
};

/* expr is null / expr is not null表达式 */
class PredNullExpr: public Expression
{
public:
  PredNullExpr(PredNullOp op, std::unique_ptr<Expression> child) : op_(op), child_(std::move(child)) {}

  RC get_value(const Tuple &tuple, Value &value) const override;

  ExprType type() const override { return ExprType::PREDNULL; }

  AttrType value_type() const override { return BOOLEANS; }

  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = BOOLEANS;
    attr.length = 1;
    attr.nullable = false;
    return attr;
  }

  std::vector<std::string> referred_tables() const override { return child_->referred_tables(); }

private:
  PredNullOp op_;
  std::unique_ptr<Expression> child_;
};

class LikeExpr: public Expression
{
public:
  LikeExpr(std::unique_ptr<Expression> child, std::string like_pattern, bool is_not_like)
    : child_(std::move(child)), like_pattern_(std::move(like_pattern)), is_not_like_(is_not_like) {};

  RC get_value(const Tuple &tuple, Value &value) const override;

  ExprType type() const override { return ExprType::LIKE; }

  AttrType value_type() const override { return BOOLEANS; }

  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = BOOLEANS;
    attr.length = 1;
    attr.nullable = false;
    return attr;
  }

  std::vector<std::string> referred_tables() const override { return child_->referred_tables(); }

private:
  RC regex_value(const std::string &string, const std::string &pattern, bool &value) const;

private:
  std::unique_ptr<Expression> child_;
  std::string like_pattern_;
  bool is_not_like_;
};

class QuantifiedCompExprSetExpr: public Expression
{
public:
  QuantifiedCompExprSetExpr(std::unique_ptr<Expression> child, QuantifiedComp op, std::vector<std::unique_ptr<Expression>> &&set)
    : child_(std::move(child)), op_(op), set_(std::move(set)) 
  {
    for (auto &expr : set_) {
      std::vector<std::string> tables = expr->referred_tables();
      referred_tables_.insert(referred_tables_.end(), std::make_move_iterator(tables.begin()), 
        std::make_move_iterator(tables.end()));
    }
    std::vector<std::string> table = child_->referred_tables();
    referred_tables_.insert(referred_tables_.end(), std::make_move_iterator(table.begin()), 
      std::make_move_iterator(table.end()));
  }

  RC get_value(const Tuple &tuple, Value &value) const override;

  ExprType type() const override { return ExprType::QUANTIFIED_COMP_EXPR_SET; }

  AttrType value_type() const override { return BOOLEANS; }

  ExprValueAttr value_attr() const override { 
    ExprValueAttr attr;
    attr.type = BOOLEANS;
    attr.length = 1;
    attr.nullable = false;
    return attr;
  }

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  std::unique_ptr<Expression> child_;
  QuantifiedComp op_;
  std::vector<std::unique_ptr<Expression>> set_;
  std::vector<std::string> referred_tables_;
};

class FunctionExpr: public Expression
{
public:
  FunctionExpr(std::unique_ptr<FunctionKernel> kernel, const FieldIdentifier &func_arg)
    : kernel_(std::move(kernel)), func_arg_(func_arg), is_const_(false) {};

  FunctionExpr(std::unique_ptr<FunctionKernel> kernel)
    : kernel_(std::move(kernel)), is_const_(true) {}

  RC get_value(const Tuple &tuple, Value &value) const override;

  ExprType type() const override { return ExprType::FUNCTION; }

  AttrType value_type() const override { return UNDEFINED; }

  ExprValueAttr value_attr() const override;

  void set_referred_table(const std::string &table) { referred_tables_.emplace_back(table); }

  std::vector<std::string> referred_tables() const override { return referred_tables_; }

private:
  std::unique_ptr<FunctionKernel> kernel_;  // 从kernel中获取值属性
  FieldIdentifier func_arg_;
  bool is_const_;
  std::vector<std::string> referred_tables_;
};