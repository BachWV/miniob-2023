#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(Table *table,std::unordered_map<int,Value> value_list)
    : table_(table), value_list_(value_list)
{}