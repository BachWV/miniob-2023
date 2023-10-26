#include "function_physical_operator.h"
#include "sql/expr/tuple_cell.h"

RC FunctionPhysicalOperator::open(Trx *trx)
{
  finish_ = false;
  RC rc   = RC::SUCCESS;
  if (!children_.empty()) {
    rc = children_.front()->open(trx);
  } else {
    // select length(sum); 可以没有child，此时自己向上返回一条
  }
  HANDLE_RC(rc);
  auto meta = FieldMeta(virtual_meta_->name(), INTS, 1, 1, true, false);
  aof_tuple_.set_new_field_meta(meta);
  return rc;
}

RC FunctionPhysicalOperator::next()
{
  RC rc;
  if (!children_.empty()) {
    rc = normal_next();
  } else {
    rc = last_oper_next();
  }
  return rc;
}

RC FunctionPhysicalOperator::normal_next()
{
  PhysicalOperator *child = children_.front().get();
  RC                rc;
  if (RC::SUCCESS == (rc = child->next())) {
    auto  tuple = child->current_tuple();
    Value be_functioned_value, out;
    if (kernel_->get_is_const()) {
      // 常量的话，不用初始化be_functioned_value
    } else {
      assert(be_functioned_field_.table());
      assert(be_functioned_field_.meta());
      auto spec = TupleCellSpec(be_functioned_field_.table_name(), be_functioned_field_.field_name());
      rc        = tuple->find_cell(spec, be_functioned_value);
      HANDLE_RC(rc);
    }

    rc = kernel_->do_func(be_functioned_value, aof_tuple_.get_new_field_value());
    HANDLE_RC(rc);

    aof_tuple_.set_original(tuple);
  }
  return rc;
}

RC FunctionPhysicalOperator::last_oper_next()
{
  RC rc;
  if (!finish_) {
    finish_ = true;
    Value v;
    // 如果是最后一个算子，那么必是常量const
    assert(kernel_->get_is_const());
    rc = kernel_->do_func(v, aof_tuple_.get_new_field_value());
  } else {
    rc = RC::RECORD_EOF;
  }
  return rc;
}

RC FunctionPhysicalOperator::close()
{
  if (children_.empty()) {
    return RC::SUCCESS;
  } else {
    return children_.front()->close();
  }
}

Tuple *FunctionPhysicalOperator::current_tuple() { return &aof_tuple_; }

FunctionPhysicalOperator::~FunctionPhysicalOperator() { delete virtual_meta_; }