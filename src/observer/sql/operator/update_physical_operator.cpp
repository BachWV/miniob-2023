#include "common/log/log.h"
#include "sql/operator/update_physical_operator.h"
#include "storage/record/record.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
#include "sql/stmt/update_stmt.h"

RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    return RC::SUCCESS;
  }

  std::unique_ptr<PhysicalOperator> &child = children_[0];
  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  rc = tuple_calculator_for_expr_in_set_->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open tuple_calculator_for_expr_in_set_: %s", strrc(rc));
    return rc;
  }

  trx_ = trx;

  return RC::SUCCESS;
}

RC UpdatePhysicalOperator::next()
{
  Record record;
  
  //----------------
  //delete
  RC rc = RC::SUCCESS;
  if (children_.empty()) {
    return RC::RECORD_EOF;
  }

  PhysicalOperator *child = children_[0].get();
  while (RC::SUCCESS == (rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (nullptr == tuple) {
      LOG_WARN("failed to get current record: %s", strrc(rc));
      return rc;
    }

    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record &record = row_tuple->record();
    //tuple已经拿到数据了，tuple存有record的指针
  
    Record record_new;

    int cell_num=row_tuple->cell_num();
    vector<Value> values_new;
    values_new.resize(cell_num);
    for(int i=0;i<cell_num;i++){
      Value value;
      row_tuple->cell_at(i,value);
      values_new.at(i)=value;
    }
    for(auto &value:value_list_)
    {
      // 计算set中的表达式值
      adaptor_for_tuple_calculator_->owns_new_tuple(tuple->get_replica());
      rc = tuple_calculator_for_expr_in_set_->next();
      if (rc != RC::SUCCESS)
      {
        LOG_WARN("failed to calculate expr in set stmt, rc=%s", strrc(rc));
        return rc;
      }
      Tuple *expr_tuple = tuple_calculator_for_expr_in_set_->current_tuple();

      auto &expr = value.second;
      Value expr_value;
      rc = expr->get_value(*expr_tuple, expr_value);
      if (rc != RC::SUCCESS)
      {
        LOG_WARN("failed to calculate expr in set stmt, rc=%s", strrc(rc));
        return rc;
      }

      values_new.at(value.first) = expr_value;
    }
    

    rc = trx_->delete_record(table_, record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to delete record: %s", strrc(rc));
      return rc;
    }

    RC rc = table_->make_record(static_cast<int>(values_new.size()), values_new.data(), record_new);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record. rc=%s", strrc(rc));
      return rc;
    }
    rc = trx_->insert_record(table_, record_new);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record: %s", strrc(rc));
      return rc;
    }
  }
  return RC::RECORD_EOF;
}

RC UpdatePhysicalOperator::close()
{
  if (!children_.empty()) {
    children_[0]->close();
  }
  tuple_calculator_for_expr_in_set_->close();
  return RC::SUCCESS;
}
