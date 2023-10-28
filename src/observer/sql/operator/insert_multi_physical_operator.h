#pragma once

#include <vector>

#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

// 顶层算子，用于create table select，在executor阶段调用
// next之前一定要先set_table
class InsertMultiPhysicalOperator: public PhysicalOperator{
public:
  InsertMultiPhysicalOperator() = default;

  virtual ~InsertMultiPhysicalOperator() = default;

  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::INSERT_MULTI;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  void set_table(Table* table){
    table_ = table;
  }

  Tuple *current_tuple() override { return nullptr; }
private:
  Table *table_ = nullptr;
  Trx* trx_;
};