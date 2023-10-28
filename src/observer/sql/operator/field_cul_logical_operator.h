#pragma once
#include "logical_operator.h"

class FieldCulLogicalOperator : public LogicalOperator
{
public:
  virtual LogicalOperatorType type() const override{
    return LogicalOperatorType::FIELD_CUL;
  }
  
  FieldCulLogicalOperator(FieldIdentifier& field_identifier, std::unique_ptr<Expression> &&cul_expr)
      : field_identifier_(field_identifier), cul_expr_(std::move(cul_expr))
  {}
  
  FieldIdentifier field_identifier_;
  std::unique_ptr<Expression> cul_expr_;
};