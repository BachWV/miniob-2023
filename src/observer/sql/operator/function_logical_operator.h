#include "logical_operator.h"
#include <utility>

class FunctionLogicalOperator: public LogicalOperator{
public:
  FunctionLogicalOperator(std::unique_ptr<FunctionKernel>&& kernel, const Field be_functioned_field, FieldMeta* virtual_meta)
    : kernel_(std::move(kernel)), be_functioned_field_(be_functioned_field), virtual_meta_(virtual_meta){}

  virtual LogicalOperatorType type() const override{
    return LogicalOperatorType::FUNCTION;
  }

  std::unique_ptr<FunctionKernel> kernel_;
  const Field be_functioned_field_;
  FieldMeta* virtual_meta_;
};