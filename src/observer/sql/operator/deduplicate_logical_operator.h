#pragma once

#include "sql/operator/logical_operator.h"
#include "storage/field/field.h"
#include <vector>

class DeduplicateLogicalOperator : public LogicalOperator
{
public:
  DeduplicateLogicalOperator(std::vector<FieldIdentifier> deduplicate_fields, bool only_put_one) 
    : deduplicate_fields_(deduplicate_fields), only_put_one_(only_put_one){};

  LogicalOperatorType type() const override { return LogicalOperatorType::DEDUPLICATE; }

  std::vector<FieldIdentifier> get_deduplicate_fields() { return deduplicate_fields_; }
  bool get_only_put_one(){
    return only_put_one_;
  }

private:
  std::vector<FieldIdentifier> deduplicate_fields_;
  bool only_put_one_;
};