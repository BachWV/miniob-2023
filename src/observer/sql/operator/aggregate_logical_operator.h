#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"
#include <vector>

enum AggregateOp;

class AggregateLogicalOperator : public LogicalOperator{
public:
	AggregateLogicalOperator(FieldMeta* new_meta ,Field agg_field, std::vector<Field> group_fields, AggregateOp op)
		: new_meta_(new_meta), agg_field_(agg_field), group_fields_(group_fields), op_(op){}
	virtual ~AggregateLogicalOperator() = default;

	LogicalOperatorType type() const override
  {
    return LogicalOperatorType::AGGREGATE;
  }

	Field get_agg_field(){
		return agg_field_;
	}

	std::vector<Field> get_group_fields(){
		return group_fields_;
	}

	AggregateOp get_op(){
		return op_;
	}

	FieldMeta* get_new_meta(){
		return new_meta_;
	}
	

private:
	FieldMeta* new_meta_;
	Field agg_field_;
	std::vector<Field> group_fields_;
	AggregateOp op_;
};