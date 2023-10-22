#pragma once

#include <vector>
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse.h"
#include "sql/expr/tuple.h"

class UpdateStmt;

/**
 * @brief 插入物理算子 更新
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table,const Value value,int field_meta_index) : table_(table), value_(value), field_meta_index_(field_meta_index) {};

  virtual ~UpdatePhysicalOperator() = default;

  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::UPDATE;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override { return nullptr; }

private:
  Table *table_ = nullptr;
  const Value value_;
  int field_meta_index_ ;
  Trx *trx_ = nullptr;
};
