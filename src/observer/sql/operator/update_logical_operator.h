#pragma once

#include <vector>

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"

/**
 * @brief 更新逻辑算子
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table,const Value value,int field_meta_index);
  virtual ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override
  {
    return LogicalOperatorType::UPDATE;
  }

  Table *table() const { return table_; }
  const Value value() const { return value_; } // fix: add return type
  //const FieldMeta *field_meta() const{ return field_meta_; }
  int field_meta_index() const { return field_meta_index_; }

private:
  Table *table_ = nullptr;
  const Value value_;
  int field_meta_index_ ;
};