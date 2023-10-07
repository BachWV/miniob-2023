#include "sql/operator/logical_operator.h"
#include "storage/field/field.h"

class SortLogicalOperator: public LogicalOperator{
public:
  SortLogicalOperator(std::vector<FieldWithOrder> order_fields): order_fields_(order_fields){}
  LogicalOperatorType type() const override
  {
    return LogicalOperatorType::SORT;
  }
  std::vector<FieldWithOrder> get_order_fields(){
    return order_fields_;
  }
private:
  std::vector<FieldWithOrder> order_fields_;
};