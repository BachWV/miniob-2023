#pragma once

#include "sql/expr/tuple.h"
#include "sql/expr/tuple_cell.h"
#include "sql/operator/physical_operator.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include <unordered_map>
#include <vector>

class DeduplicateAggPhysicalOperator: public PhysicalOperator{
public:
	DeduplicateAggPhysicalOperator(std::vector<Field> group_by_fields, bool only_put_one)
		: group_by_fields_(group_by_fields), only_put_one_(only_put_one){}
  virtual PhysicalOperatorType type() const override{
    return PhysicalOperatorType::DEDUPLICATE_AGG;
  }

  virtual RC open(Trx *trx) override;
  virtual RC next() override;
  virtual RC close() override;

  virtual Tuple *current_tuple() override;

  RC set_group_by_value(const Tuple* tuple, std::vector<Value>& cur_group_by_value);

  bool cmp_group_by_fields(const Tuple* cur_tuple, const std::vector<Value>& cur_group_by_value);

private:
	// std::unordered_map<std::vector<Field>, bool> unique_map;
  bool only_put_one_;   // 应该写两个算子，但是我懒, 这个看gbf的empty就能看出来吧，算了，显式写出来好理解

  bool finish_;

	std::vector<Field> group_by_fields_;	// 这个就是schema
  std::vector<Value> cur_group_by_value_;
  Tuple* curr_tuple_;
};