#pragma once
#include "logical_operator.h"

class FieldCulLogicalOperator : public LogicalOperator
{
public:
  virtual LogicalOperatorType type() const override{
    return LogicalOperatorType::FIELD_CUL;
  }
  
  FieldCulLogicalOperator(FieldMeta *virtual_meta, std::unique_ptr<Expression> &&cul_expr)
      : virtual_meta_(virtual_meta), cul_expr_(std::move(cul_expr))
  {}
  
  FieldMeta                  *virtual_meta_;  // delete
  std::unique_ptr<Expression> cul_expr_;
};