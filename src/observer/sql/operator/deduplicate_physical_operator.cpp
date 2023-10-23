#include "sql/operator/deduplicate_physical_operator.h"
#include "common/log/log.h"
#include "common/rc.h"

RC DeduplicateAggPhysicalOperator::open(Trx *trx){
	if (children_.size() != 1) {
    LOG_WARN("deduplicate agg operator must has one child");
    return RC::INTERNAL;
  }

	finish_ = false;

  	auto child = children_[0].get();
	auto rc = child->open(trx);
	if(rc == RC::EMPTY){
		finish_ = true;
	}else if(rc != RC::SUCCESS){
		return rc;
	}

	cur_group_by_value_.resize(group_by_fields_.size());

	if(RC::SUCCESS == (rc = child->next())){
		auto tuple = child->current_tuple();
		rc = set_group_by_value(tuple, cur_group_by_value_);
	}

	return RC::SUCCESS;
}

RC DeduplicateAggPhysicalOperator::next(){
  	RC rc = RC::SUCCESS;
  	PhysicalOperator *child = children_.front().get();

	// 无group by，只输出第一个的判断
	if(finish_){
		return RC::RECORD_EOF;
	}
	if(only_put_one_){
		finish_ = true;
		rc = child->next();
		HANDLE_RC(rc);

		curr_tuple_ = child->current_tuple();
		return rc;
	}

	auto tuple = child->current_tuple();
	rc = set_group_by_value(tuple, cur_group_by_value_);
	HANDLE_RC(rc);
	curr_tuple_ = tuple;

	// 有group by，一类gbk只输出一个
	while (RC::SUCCESS == (rc = child->next())){
		tuple = child->current_tuple();
		if(!cmp_group_by_fields(tuple, cur_group_by_value_)){
			break;
		}
	}

	if(rc == RC::RECORD_EOF){
		finish_ = true;
	}
	
	return RC::SUCCESS;
}

Tuple* DeduplicateAggPhysicalOperator::current_tuple()
{
	return curr_tuple_;
}

RC DeduplicateAggPhysicalOperator::close(){
	return children_[0]->close();
}

bool DeduplicateAggPhysicalOperator::cmp_group_by_fields(const Tuple* cur_tuple, const std::vector<Value>& cur_group_by_value){
	assert(cur_tuple);
	// 有group by
	for(int i = 0; i < group_by_fields_.size(); i++){
		Value v;
		TupleCellSpec spec(group_by_fields_[i].table_name(), group_by_fields_[i].field_name());
		auto rc = cur_tuple->find_cell(spec, v);
		if(rc != RC::SUCCESS){
			// 不应该发生，测试阶段，直接kill
			assert(0);
		}
		if(0 != v.compare(cur_group_by_value[i])){
			return false;
		}
	}

	return true;
}

RC DeduplicateAggPhysicalOperator::set_group_by_value(const Tuple* tuple, std::vector<Value>& cur_group_by_value){
	for(int i = 0; i < group_by_fields_.size(); i++){
		TupleCellSpec tcs(group_by_fields_[i].table_name(), group_by_fields_[i].field_name());
		auto rc = tuple->find_cell(tcs, cur_group_by_value[i]);
		HANDLE_ASSERT_RC(rc);
	}
	return RC::SUCCESS;
}