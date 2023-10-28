#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"
#include "storage/field/field.h"
#include <vector>

enum AggregateOp;

class AggregateLogicalOperator : public LogicalOperator{
public:
	AggregateLogicalOperator(FieldIdentifier fid ,std::string virtual_name, std::vector<FieldIdentifier> group_fids, AggregateOp op)
		: virtual_name_(virtual_name), fid_(fid), group_fids_(group_fids), op_(op){}
	virtual ~AggregateLogicalOperator() = default;

	LogicalOperatorType type() const override
  {
    return LogicalOperatorType::AGGREGATE;
  }

	FieldIdentifier fid_;
	std::string virtual_name_;
	std::vector<FieldIdentifier> group_fids_;
	AggregateOp op_;
};