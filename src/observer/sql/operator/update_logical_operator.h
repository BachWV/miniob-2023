#pragma once

#include <vector>

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"

/**
 * @brief 更新逻辑算子
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table, std::unordered_map<int, std::unique_ptr<Expression>> &&value_list,
    std::vector<std::unique_ptr<LogicalOperator>> &&subquerys_in_set_stmts);
  virtual ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override
  {
    return LogicalOperatorType::UPDATE;
  }

  Table *table() const { return table_; }
  std::unordered_map<int, std::unique_ptr<Expression>> &value_list() { return value_list_; }
  std::vector<std::unique_ptr<LogicalOperator>> &subquerys_in_set_stmts() { return subquerys_in_set_stmts_; }

private:
  Table *table_ = nullptr;
  std::unordered_map<int, std::unique_ptr<Expression>> value_list_;

  /* 
   * 如果有子查询，则存在下面的Apply算子集合：
   * 它们是特殊的，不存在于Update的算子树中，而是UpdatePhysicalOperator算子的内部实现，所以在这里维护
   */
  std::vector<std::unique_ptr<LogicalOperator>> subquerys_in_set_stmts_;  // set表达式中的子查询
};