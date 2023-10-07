#pragma once

#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include <functional>
#include <vector>

class Trx;

/**
 * @brief 物理算子，删除
 * @ingroup PhysicalOperator
 */
class SortPhysicalOperator : public PhysicalOperator
{
public:
  SortPhysicalOperator(std::vector<FieldWithOrder> order_fields) : order_fields_(order_fields)
  {}

  virtual ~SortPhysicalOperator() = default;

  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::SORT;
  }

  // 获得下层算子所有的tuple并排好序
  RC open(Trx *trx) override;
  // 根据current_index来获得当前的tuple
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  bool generate_cmp_function();

private:
  Trx *trx_ = nullptr;

  int current_tuple_index_{0};
  std::vector<Tuple*> tuples;    // 子算子的所有tuple
  std::vector<FieldWithOrder> order_fields_;
  std::vector<std::function<bool(const Value&, const Value&)>> cmp_functions;   // 根据order_fields_生成的比较函数
};
