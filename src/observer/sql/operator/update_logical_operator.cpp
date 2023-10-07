#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(Table *table,const Value value,int field_meta_index)
    : table_(table), value_(value),field_meta_index_(field_meta_index)
{
}