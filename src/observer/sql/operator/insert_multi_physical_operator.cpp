#include "sql/operator/insert_multi_physical_operator.h"
#include <cassert>
#include "common/rc.h"
#include "sql/expr/tuple.h"
#include "sql/parser/value.h"

RC InsertMultiPhysicalOperator::open(Trx *trx){
  RC rc = RC::SUCCESS;
  trx_ = trx;   // 对事务会不会有影响
  assert(children_.size() == 1);
  return children_.front()->open(trx);
}

RC InsertMultiPhysicalOperator::next(){
  assert(table_);
  auto child = children_.front().get();
  RC rc= RC::SUCCESS;
  std::vector<Value> values;
  if(RC::SUCCESS == (rc = child->next())){
    auto tuple = child->current_tuple();
    values.resize(tuple->cell_num());
    for(int i = 0; i < tuple->cell_num(); i++){
      rc = tuple->cell_at(i, values[i]);
      HANDLE_RC(rc);
    }
    Record record;
    rc = table_->make_record(static_cast<int>(values.size()), values.data(), record);
    HANDLE_RC(rc);
    rc = trx_->insert_record(table_, record);
  }
  return rc;
}

RC InsertMultiPhysicalOperator::close(){
  return children_.front()->close();
}