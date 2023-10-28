#include "aggregate_physical_operator.h"
#include "common/lang/string.h"
#include "common/rc.h"
#include "event/sql_debug.h"
#include "sql/expr/tuple_cell.h"

RC AggregatePhysicalOperator::open(Trx *trx){
	if (children_.size() != 1) {
    LOG_WARN("aggregate operator must has one child");
    return RC::INTERNAL;
  }
  	all_index_ = -1;
    avg_not_null_count_ = 0;
  	auto child = children_[0].get();
	auto rc = child->open(trx);
	HANDLE_RC(rc);
	
	rc = init_curr_group(true);
	if(rc == RC::EMPTY){
		// 无数据
		return RC::EMPTY;
	}
	// 有group by的先聚集一个gbk
	// 无group by的直接聚集完全部
	rc = do_group_agg();
	HANDLE_RC(rc)

	while(!is_last_group_){
		assert(!group_by_fids_.empty());

		auto rc = init_curr_group(false);
		HANDLE_RC(rc);

		rc = do_group_agg();
		HANDLE_RC(rc);
	}
	
	return RC::SUCCESS;
}

// 一个next：①拿到一个group里面的所有tuple；②维护计数器，输出这个group里面还剩多少条
RC AggregatePhysicalOperator::next(){
	auto size = all_aof_tuples_.size();
	all_index_++;
	if(all_index_ < size){
		// 不是当前组最后一个
		return RC::SUCCESS;
	}else{
		return RC::RECORD_EOF;
	}
}

// [0, size)
Tuple* AggregatePhysicalOperator::current_tuple(){
	return &all_aof_tuples_[all_index_];
}

RC AggregatePhysicalOperator::close(){
	all_aof_tuples_.clear();
	return children_.front()->close();
}

bool AggregatePhysicalOperator::cmp_group_by_fields(const Tuple* cur_tuple, const std::vector<Value>& cur_group_by_value){
	if(group_by_fids_.empty()){
		// 无group by聚集函数，无视gbf的比较，直接全部累加
		return true;
	}

	// 有group by
	for(int i = 0; i < group_by_fids_.size(); i++){
		Value v;
		TupleCellSpec spec(group_by_fids_[i].table_name(), group_by_fids_[i].field_name());
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


RC AggregatePhysicalOperator::set_group_by_value(const Tuple* tuple, std::vector<Value>& cur_group_by_value){
	for(int i = 0; i < group_by_fids_.size(); i++){
		TupleCellSpec tcs(group_by_fids_[i].table_name(), group_by_fids_[i].field_name());
		auto rc = tuple->find_cell(tcs, cur_group_by_value[i]);
		HANDLE_ASSERT_RC(rc);
	}
	return RC::SUCCESS;
}

RC AggregatePhysicalOperator::get_value(Tuple* tuple, Value& v){
	RC rc;
	if(0 == common::str_equal("COUNT(*)", virtual_name_.c_str())){
		rc = tuple->cell_at(0, v);
	}else{
		auto spec = TupleCellSpec(agg_fid_.table_name(), agg_fid_.field_name());
		rc = tuple->find_cell(spec, v);
	}
	return rc;
}

RC AggregatePhysicalOperator::init_curr_group(bool first){
	PhysicalOperator *child = children_.front().get();
	aof_tuples_.clear();
	avg_not_null_count_ = 0;

	RC rc;
	Tuple* tuple = nullptr;
	if(first){
		// Open时的初始化，第一次open算子后，必须先next，才能拿到curr tuple
		if(RC::SUCCESS == (rc = child->next())){
			tuple = child->current_tuple();
		}else{
			return RC::EMPTY;
		}
		
	}else{
		// 正常状态下的init，直接设置当前curr tuple就可以，
		// 使用next会忽略掉当前gbk的第一个值（因为上一轮末尾已经将其设置到了curr tuple）
		tuple = child->current_tuple();
	}

	rc = set_group_by_value(tuple, cur_group_by_value_);
	HANDLE_RC(rc);

	Value v;
	rc = get_value(tuple, v);
	HANDLE_RC(rc);
	do_single_aggregate(v);
	auto meta = FieldMeta(virtual_name_.c_str(), INTS , 1,1, true, false);
	aof_tuples_.push_back(AddOneFieldTuple(tuple, meta));

	return RC::SUCCESS;
}

RC AggregatePhysicalOperator::do_group_agg(){
	RC rc;
	PhysicalOperator *child = children_.front().get();
	// avg_not_null_count_ = 0;

	// 用do while其实更好
	while(RC::SUCCESS == (rc = child->next())){
		Tuple *tuple = child->current_tuple();
		if(cmp_group_by_fields(tuple, cur_group_by_value_)){
			// 当前group by字段还在执行
			Value v;
			rc = get_value(tuple, v);
			HANDLE_RC(rc);

			do_single_aggregate(v);

			auto meta = FieldMeta(virtual_name_.c_str(), INTS , 1,1, true, false);
			aof_tuples_.push_back(AddOneFieldTuple(tuple, meta));
		}else{
			break;
		}
	}

	if(rc == RC::RECORD_EOF){
		is_last_group_ = true;
	}

	// 本轮group by已经完成
	if(op_ == AGG_AVG){
		if(curr_group_agg_value_.is_null_value()){
			// empty
		}else{
			auto v = curr_group_agg_value_.get_float();
			assert(avg_not_null_count_ != 0);
			sql_debug("avg_not_null_count_ = %d", avg_not_null_count_);
			curr_group_agg_value_.set_float(v / avg_not_null_count_);
		}
	}

	for(auto & t: aof_tuples_){
		t.set_new_field_value(curr_group_agg_value_);
	}

	for(const auto& tuple: aof_tuples_){
		all_aof_tuples_.push_back(tuple);
	}

	return RC::SUCCESS;
}

void AggregatePhysicalOperator::do_single_aggregate(const Value& curr_value){
	switch (op_) {
	case AGG_MAX:{
			if(aof_tuples_.empty()){	// 第一次进入本轮GroupBy，设置初始值
				curr_group_agg_value_ = curr_value;	// 如果本列全是null的话，最终结果也是null
			}
			if(curr_value.is_null_value() ){
				return;
			}

			if( 0 < curr_value.compare(curr_group_agg_value_))
				curr_group_agg_value_ = curr_value;
			return;
		};break;

	case AGG_MIN:{
			if(aof_tuples_.empty()){	// 第一次进入本轮GroupBy，设置初始值
				curr_group_agg_value_ = curr_value;
			}
			if(curr_value.is_null_value()){
				return;
			}

			if( 0 > curr_value.compare(curr_group_agg_value_))
				curr_group_agg_value_ = curr_value;
			return;
		};break;

	case AGG_COUNT:{
			if(aof_tuples_.empty()){
				curr_group_agg_value_.set_int(0);
			}

			if( (common::str_equal(virtual_name_.c_str(), "COUNT(*)") != 0) && curr_value.is_null_value()){	//忽视NULL
				return;
			}

			auto v = curr_group_agg_value_.get_int();
			curr_group_agg_value_.set_int(v+1);
			return;
			
		};break;

	case AGG_AVG:{
		// 不能处理字符串，bool, date
		assert(curr_value.attr_type() != BOOLEANS);
		assert(curr_value.attr_type() != DATES);
		assert(curr_value.attr_type() != CHARS);
		
			if(aof_tuples_.empty()){	// 第一次进入本轮GroupBy，设置初始值
				curr_group_agg_value_ = curr_value;	// 如果本列全是null的话（第一个是NULL），最终结果也是null
				if(!curr_value.is_null_value()){
					avg_not_null_count_++;
				}
				return;
			}

			if(curr_value.is_null_value()){	// NULL忽略
				return;
			}

			if(curr_group_agg_value_.is_null_value()){	// cgav可能是NULL
				curr_group_agg_value_ = curr_value;
				avg_not_null_count_++;
				return;
			}

			// avg的value type一定是Float
			auto v1 = curr_value.get_float();
			auto v2 = curr_group_agg_value_.get_float();
			curr_group_agg_value_.set_float(v1+v2);
			avg_not_null_count_++;
			return;
		};break;
	case AGG_SUM:{
			assert(curr_value.attr_type() != BOOLEANS);
			assert(curr_value.attr_type() != DATES);
			assert(curr_value.attr_type() != CHARS);
	
			if(aof_tuples_.empty()){	// 第一次进入本轮GroupBy，设置初始值
				curr_group_agg_value_ = curr_value;	// 如果本列全是null的话（第一个是NULL），最终结果也是null
				return;
			}

			if(curr_value.is_null_value()){	// NULL忽略
				return;
			}

			if(curr_group_agg_value_.is_null_value()){	// cgav可能是NULL
				curr_group_agg_value_ = curr_value;
				return;
			}

			// avg的value type一定是Float
			auto v1 = curr_value.get_float();
			auto v2 = curr_group_agg_value_.get_float();
			curr_group_agg_value_.set_float(v1+v2);
			return;
		};break;
	}
}