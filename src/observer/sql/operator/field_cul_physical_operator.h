#pragma once
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "storage/field/field_meta.h"

class FieldCulPhysicalOperator : public PhysicalOperator
{
public:
  FieldCulPhysicalOperator(FieldIdentifier& fi, std::unique_ptr<Expression> &&cul_expr)
      : cul_expr_(std::move(cul_expr)){
        virtual_meta_ = FieldMeta(fi.field_name().c_str(),INTS, 1, 1, true, false);
      }
  ~FieldCulPhysicalOperator() = default;
  PhysicalOperatorType type() const override { return PhysicalOperatorType::FIELD_CUL; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  RC normal_next();
  RC last_oper_next();


  Tuple *current_tuple() override;

private:
  AddOneFieldTuple            aof_tuple_;
  FieldMeta                   virtual_meta_;  // delete
  std::unique_ptr<Expression> cul_expr_;
  bool finish_;
};