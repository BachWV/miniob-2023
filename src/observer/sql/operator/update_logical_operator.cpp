#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(Table *table,std::unordered_map<int, std::unique_ptr<Expression>> &&value_list,
    std::vector<std::unique_ptr<LogicalOperator>> &&subquerys_in_set_stmts)
    : table_(table), value_list_(std::move(value_list)), subquerys_in_set_stmts_(std::move(subquerys_in_set_stmts))
{}