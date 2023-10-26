#pragma once

#include "common/rc.h"
#include "sql/expr/tuple.h"
#include "sql/expr/tuple_cell.h"
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/parser/value.h"
#include <cstring>
#include <functional>
#include <vector>

enum AggregateOp;


class AggregatePhysicalOperator: public PhysicalOperator{
public:
	AggregatePhysicalOperator(Field agg_field ,FieldMeta* new_meta, std::vector<Field> group_by_fields, AggregateOp op)
		: agg_field_(agg_field), new_meta_(new_meta), group_by_fields_(group_by_fields), op_(op){
			if(!group_by_fields.empty()){
				cur_group_by_value_.resize(group_by_fields.size());
			}
		}
	~AggregatePhysicalOperator();

	PhysicalOperatorType type() const override{
		return PhysicalOperatorType::AGGREGATE;
	}

	virtual RC open(Trx *trx) override;
  virtual RC next() override;
  virtual RC close() override;

  virtual Tuple *current_tuple() override;

	// 有group by：比较cgbv字段
	// 无group by：返回true，让外层do_group_agg()直接agg完所有的子tuple
	bool cmp_group_by_fields(const Tuple* cur_tuple, const std::vector<Value>& cur_group_by_value);

	RC set_group_by_value(const Tuple* tuple, std::vector<Value>& cur_group_by_value);

	// 使用前清空aof_tuples!(clear)
	// 外层还要判断一下是不是avg，avg要除以个数
	void do_single_aggregate(const Value& curr_value);

	// TODO: 对count(*)加一个特判。这里写的不好，不应该把COUNT和AGG耦合这么多。正常应该每个agg函数单独开一个算子，但是我懒
	RC get_value(Tuple* tuple, Value& v);

	RC init_curr_group(bool first);

	RC do_group_agg();

private:
	Field agg_field_;
	FieldMeta* new_meta_;
	AggregateOp op_;

	std::vector<Field> group_by_fields_;

	std::vector<AddOneFieldTuple> aof_tuples_;// 就是curr_group_tuples_+这一轮的agg_value
	// 只保证每个gb-key轮次的状态
	std::vector<Value> cur_group_by_value_;
	Value curr_group_agg_value_;

	bool is_last_group_{false};

	int all_index_;
	std::vector<AddOneFieldTuple> all_aof_tuples_;

	int avg_not_null_count_ ;
};