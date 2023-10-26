#pragma once
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"

class FieldCulPhysicalOperator : public PhysicalOperator
{
public:
  FieldCulPhysicalOperator(FieldMeta *virtual_meta, std::unique_ptr<Expression> &&cul_expr)
      : virtual_meta_(virtual_meta), cul_expr_(std::move(cul_expr)){}
  ~FieldCulPhysicalOperator();
  PhysicalOperatorType type() const override { return PhysicalOperatorType::FIELD_CUL; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

private:
  AddOneFieldTuple            aof_tuple_;
  FieldMeta                  *virtual_meta_;  // delete
  std::unique_ptr<Expression> cul_expr_;
};