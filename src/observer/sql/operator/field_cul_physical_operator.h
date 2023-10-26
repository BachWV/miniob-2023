#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"

class FieldCulPhysicalOperator: public PhysicalOperator{
public:
  ~FieldCulPhysicalOperator();
  PhysicalOperatorType type() const override{
    return PhysicalOperatorType::FIELD_CUL;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  RC normal_next();
  RC last_oper_next();

  Tuple *current_tuple() override;

private:
  AddOneFieldTuple aof_tuple_;
  FieldMeta* virtual_meta_; //delete
};