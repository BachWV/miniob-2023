#include "field_cul_physical_operator.h"

RC  FieldCulPhysicalOperator::open(Trx *trx){
  finish_ = false;
  aof_tuple_.set_new_field_meta(virtual_meta_);
  RC rc = RC::SUCCESS;
  if (!children_.empty()) {
    rc = children_.front()->open(trx);
  } else {
    // select length(sum); 可以没有child，此时自己向上返回一条
  }
  return rc;
}

RC  FieldCulPhysicalOperator::next(){
  RC rc;
  if (!children_.empty()) {
    rc = normal_next();
  } else {
    rc = last_oper_next();
  }
  return rc;
}

RC FieldCulPhysicalOperator::normal_next(){
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

RC FieldCulPhysicalOperator::last_oper_next(){
  RC rc;
  if (!finish_) {
    finish_ = true;
    Value v;
    // 如果是最后一个算子，那么必是常量const
    // 第一个参数传入aof是因为占位符
    
    rc = cul_expr_->get_value(aof_tuple_, aof_tuple_.get_new_field_value());
  } else {
    rc = RC::RECORD_EOF;
  }
  return rc;
}


RC  FieldCulPhysicalOperator::close(){
  if (children_.empty()) {
    return RC::SUCCESS;
  } else {
    return children_.front()->close();
  }
}

Tuple * FieldCulPhysicalOperator::current_tuple(){
  return &aof_tuple_;
}