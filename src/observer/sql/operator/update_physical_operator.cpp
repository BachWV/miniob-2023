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
    Record record_need_delete;
    Record record_new;

    TableMeta table_meta = table_->table_meta();
    int cell_num= table_meta.field_num()-table_meta.sys_field_num();
    //int cell_num=row_tuple->cell_num();
    vector<Value> values_new;
    vector<Value> values_need_delete;
    values_new.resize(cell_num);
    values_need_delete.resize(cell_num);
    for(int i = 0;i < cell_num ; i++){
      Value value;
      row_tuple->cell_at(i + table_meta.sys_field_num(),value);
      values_new.at(i) = value;
      values_need_delete.at(i) = value;
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
      // const FieldMeta *fm = table_meta.field(value.first);
      // char *field_data = record.data() + fm->offset(); 
     // memcpy(field_data, expr_value.data(), fm->len());


      values_new.at(value.first - table_meta.sys_field_num()) = expr_value;
    }

    RC rc = table_->make_record(static_cast<int>(values_new.size()), values_new.data(), record_new);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record. rc=%s", strrc(rc));
      return rc;
    } 

    rc = trx_->visit_record(table_, record, true);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to visit record: %s", strrc(rc));
      //说明其他正在修改，直接返回
      return rc;
    }

    std::vector<std::vector<int>> unique_field_list_list;
    bool has_unique_index = table_meta.has_unique_index();
    bool need_to_scan = has_unique_index;
    bool need_to_roll_back = false;

    if (has_unique_index)  table_meta.find_unique_index_field_vector(unique_field_list_list);

    if (has_unique_index){ 
      //对null进行特判
      for(std::vector<int> unique_field_list : unique_field_list_list){
        for(int unique_field : unique_field_list){
          if(record.is_null_value(unique_field)){
            need_to_scan=false;//如果有一个是null，就不需要去重了，直接插入
            break;
          }
        }
        if(need_to_scan==false) break;
      }
    }

    if(need_to_scan){
      RecordFileScanner scanner;
      Record record_tmp;
      rc = table_->get_record_scanner(scanner, trx_, true/*readonly*/);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create scanner . table=%s,, rc=%s", name(), strrc(rc));
        return rc;
      }
      while (scanner.has_next()) {
        rc = scanner.next(record_tmp);
        RowTuple tuple_tmp;
        tuple_tmp.set_schema(table_, table_meta.field_metas());
        tuple_tmp.set_record(&record_tmp);
       
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to scan records . table=%s,, rc=%s",name(), strrc(rc));
          return rc;
        }

        for(std::vector<int> unique_field_list : unique_field_list_list){
          bool flag = true;
          for(int unique_field : unique_field_list){
            Value value1;
            Value value2=values_new.at(unique_field);
            tuple_tmp.cell_at(unique_field,value1);
            if(value1.compare(value2)!=0){
              flag=false;
              break;
            }
          }
          if(flag){//全部相等,跳过插入,update failure,回滚
            need_to_roll_back=true;
            break;
          }
        }//比较结束
        if(need_to_roll_back) break;
      }//扫描结束
    
    }//need_to_scan结束


    if(need_to_roll_back){
      LOG_WARN("update fail: duplicate key ,failed to insert record: %s", strrc(rc));
      // RC rc2 = table_->make_record(static_cast<int>(values_need_delete.size()),values_need_delete.data(),record_need_delete);
      // if(rc2!=RC::SUCCESS)
      // {
      //   LOG_WARN("update rollback :failed to make deteled record. rc=%s", strrc(rc2));
      //   return rc2;
      // }
      // RC rc3 = trx_->insert_record(table_, record_need_delete);
      // if(rc3!=RC::SUCCESS)
      // {
      //   LOG_WARN("update rollback :failed to insert deteled record. rc=%s", strrc(rc2));
      //   return rc3;
      // }
      // LOG_WARN("update rollback :SUCCESS,Deleted record insert success. rc=%s", strrc(rc));
      return RC::RECORD_DUPLICATE_KEY;
    }

    int data_len=table_meta.record_size();
    auto record_updater = [&record_new,&data_len](Record &record_rid) {
        char *field_data = record_rid.data();//这是record.rid传入的record
        memcpy(field_data, record_new.data(), sizeof(data_len));
    };

    rc = table_->visit_record(record.rid(), false/*readonly*/, record_updater);

    // rc = trx_->insert_record(table_, record_new);
    // if (rc != RC::SUCCESS) {
    //   LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
    // }else{
    //   LOG_DEBUG("update success: insert new record success. rc=%s", strrc(rc));
    //   continue;
    // }

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
