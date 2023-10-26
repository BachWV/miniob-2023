#include "sql/expr/tuple.h"
#include "sql/function_kernel/function_kernel.h"
#include "sql/operator/physical_operator.h"
#include <memory>
#include <string>

class FunctionPhysicalOperator : public PhysicalOperator
{
public:
  FunctionPhysicalOperator(std::unique_ptr<FunctionKernel>&& kernel, const Field be_functioned_field, FieldMeta* virtual_meta)
    : kernel_(std::move(kernel)), be_functioned_field_(be_functioned_field), virtual_meta_(virtual_meta){}

  ~FunctionPhysicalOperator();
  PhysicalOperatorType type() const override{
    return PhysicalOperatorType::FUNCTION;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  RC normal_next();
  RC last_oper_next();

  Tuple *current_tuple() override;

private:
  std::unique_ptr<FunctionKernel> kernel_;
  AddOneFieldTuple aof_tuple_;
  const Field be_functioned_field_;
  FieldMeta* virtual_meta_; //delete

  bool finish_;
};