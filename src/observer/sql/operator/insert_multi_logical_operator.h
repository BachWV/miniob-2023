#pragma once

#include <vector>

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"

// 啥都不存，事务是顶层open的时候传入，table是顶层开始工作的时候传入
class InsertMultiLogicalOperator: public LogicalOperator{
public:
  InsertMultiLogicalOperator() = default;
  virtual ~InsertMultiLogicalOperator() = default;

  LogicalOperatorType type() const override
  {
    return LogicalOperatorType::INSERT_MULTI;
  }
};