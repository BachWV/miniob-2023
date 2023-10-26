#include "field_cul_physical_operator.h"

RC  FieldCulPhysicalOperator::open(Trx *trx){
  aof_tuple_.set_new_field_meta(*virtual_meta_);
  assert(!children_.empty());
  return children_.front()->open(trx);
}

RC  FieldCulPhysicalOperator::next(){
  RC rc = RC::SUCCESS;
  PhysicalOperator *oper = children_.front().get();

  if (RC::SUCCESS == (rc = oper->next())) {
    Tuple *tuple = oper->current_tuple();
    if (nullptr == tuple) {
      rc = RC::INTERNAL;
      LOG_WARN("failed to get tuple from operator");
    }

    Value value;
    rc = cul_expr_->get_value(*tuple, value);
    if (rc != RC::SUCCESS) {
      return rc;
    }

    aof_tuple_.set_original(tuple);
    aof_tuple_.set_new_field_value(value);
  }
  return rc;
}

RC  FieldCulPhysicalOperator::close(){
  return children_.front()->close();
}

Tuple * FieldCulPhysicalOperator::current_tuple(){
  return &aof_tuple_;
}

FieldCulPhysicalOperator::~FieldCulPhysicalOperator(){
  delete virtual_meta_;
}