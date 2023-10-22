#include "aggregate_physical_operator.h"
#include "common/lang/string.h"
#include "common/rc.h"
#include "sql/expr/tuple_cell.h"

RC AggregatePhysicalOperator::open(Trx *trx){
	if (children_.size() != 1) {
    LOG_WARN("aggregate operator must has one child");
    return RC::INTERNAL;
  }

  auto child = children_[0].get();
	auto rc = child->open(trx);
	HANDLE_RC(rc);

	rc = init_curr_group(true);
	HANDLE_RC(rc);

	// 有group by的先聚集一个gbk
	// 无group by的直接聚集完全部
	rc = do_group_agg();
	HANDLE_RC(rc)
	
	return rc;
}

// 一个next：①拿到一个group里面的所有tuple；②维护计数器，输出这个group里面还剩多少条
RC AggregatePhysicalOperator::next(){
	auto size = aof_tuples_.size();
	curr_group_count_++;

  if(curr_group_count_ < size){
		// 不是当前组最后一个
		return RC::SUCCESS;

	}else if(curr_group_count_ == size && is_last_group_){
		// 当前组输出完成，是最后一组
		return RC::RECORD_EOF;

	}else{
		// 当前组输出完成，但不是最后一组，无group by的聚集函数不会进入这个循环
		assert(!group_by_fields_.empty());

		auto rc = init_curr_group(false);
		HANDLE_RC(rc);

		rc = do_group_agg();
		HANDLE_RC(rc);
		return RC::SUCCESS;
	}
}

// [0, size)
Tuple* AggregatePhysicalOperator::current_tuple(){
	return &aof_tuples_[curr_group_count_];
}

RC AggregatePhysicalOperator::close(){
	// new_meta在select logical generator的时候在堆上生成
	// 该虚拟字段的生命周期和Agg算子绑定
	delete new_meta_;

	if( op_ == AGG_COUNT && common::str_equal("COUNT(*)", agg_field_.field_name())){
		delete agg_field_.meta();
	}

	return children_.front()->close();
}

bool AggregatePhysicalOperator::cmp_group_by_fields(const Tuple* cur_tuple, const std::vector<Value>& cur_group_by_value){
		if(group_by_fields_.empty()){
			// 无group by聚集函数，无视gbf的比较，直接全部累加
			return true;
		}

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

RC AggregatePhysicalOperator::set_group_by_value(const Tuple* tuple, std::vector<Value>& cur_group_by_value){
	for(int i = 0; i < group_by_fields_.size(); i++){
		TupleCellSpec tcs(group_by_fields_[i].table_name(), group_by_fields_[i].field_name());
		auto rc = tuple->find_cell(tcs, cur_group_by_value[i]);
		HANDLE_ASSERT_RC(rc);
	}
	return RC::SUCCESS;
}

RC AggregatePhysicalOperator::get_value(Tuple* tuple, Value& v){
	RC rc;
	if(0 == strcmp("COUNT(*)", new_meta_->name())){
		rc = tuple->cell_at(0, v);
	}else{
		auto spec = TupleCellSpec(agg_field_.table_name(), agg_field_.field_name());
		rc = tuple->find_cell(spec, v);
	}
	return rc;
}

RC AggregatePhysicalOperator::init_curr_group(bool first){
	PhysicalOperator *child = children_.front().get();
	aof_tuples_.clear();
	curr_group_count_ = 0;

	RC rc;
	Tuple* tuple;
	if(first){
		// Open时的初始化，第一次open算子后，必须先next，才能拿到curr tuple
		if(RC::SUCCESS == (rc = child->next())){
			tuple = child->current_tuple();
			curr_group_count_ = -1;
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
	aof_tuples_.push_back(AddOneFieldTuple(tuple, *new_meta_));

	return RC::SUCCESS;
}

RC AggregatePhysicalOperator::do_group_agg(){
	RC rc;
	PhysicalOperator *child = children_.front().get();

	// 用do while其实更好
	while(RC::SUCCESS == (rc = child->next())){
		Tuple *tuple = child->current_tuple();
		if(cmp_group_by_fields(tuple, cur_group_by_value_)){
			// 当前group by字段还在执行
			Value v;
			rc = get_value(tuple, v);
			HANDLE_RC(rc);

			do_single_aggregate(v);

			aof_tuples_.push_back(AddOneFieldTuple(tuple, *new_meta_));
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
			curr_group_agg_value_.set_float(v / aof_tuples_.size());
		}
	}

	for(auto & t: aof_tuples_){
		t.set_new_field_value(curr_group_agg_value_);
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

			// TODO: 这里要改，
			if( (strcmp(new_meta_->name(), "COUNT(*)") != 0) && curr_value.is_null_value()){	//忽视NULL
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