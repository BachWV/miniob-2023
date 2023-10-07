#include "sql/operator/sort_physical_operator.h"

RC SortPhysicalOperator::open(Trx *trx){
	RC rc = RC::SUCCESS;
	PhysicalOperator *oper = children_.front().get();
	while(RC::SUCCESS == (rc = oper->next())){
		Tuple* tuple = oper->current_tuple();
		tuples.push_back(tuple);
	}
	generate_cmp_function();

	return RC::SUCCESS;
}

RC SortPhysicalOperator::next(){
	return RC::SUCCESS;
}


RC SortPhysicalOperator::close(){
	return RC::SUCCESS;
}

Tuple* SortPhysicalOperator::current_tuple(){
	return nullptr;
}

bool SortPhysicalOperator::generate_cmp_function(){
	for(auto& fwo: order_fields_){
		
	}
	return true;
}