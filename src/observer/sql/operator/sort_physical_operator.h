#pragma once

#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "sql/expr/tuple.h"
#include "common/lang/comparator.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
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
  SortPhysicalOperator(std::vector<FieldWithOrder> order_fields) : order_fields_(order_fields) {}

  virtual ~SortPhysicalOperator();

  PhysicalOperatorType type() const override { return PhysicalOperatorType::SORT; }

  // 获得下层算子所有的tuple并排好序
  RC open(Trx *trx) override;
  // 根据current_index来获得当前的tuple
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

  bool generate_cmp_functions();
  bool cmp_function(Tuple * const &l_tuple, Tuple * const &r_tuple);

  class CmpFunctor;

private:
  Trx *trx_ = nullptr;

  long long                            curr_index_;
  std::vector<Tuple *>           tuples_;  // 子算子的所有tuple
  std::vector<FieldWithOrder>    order_fields_;
  std::vector<std::function<bool(const Tuple *, const Tuple *, bool &equal)>>
      cmp_functions_;  // 根据order_fields_生成的比较函数
  
};
