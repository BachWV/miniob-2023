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
    Record record_need_delete;
    Record record_new;

    int cell_num=row_tuple->cell_num();
    vector<Value> values_new;
    vector<Value> values_need_delete;
    values_new.resize(cell_num);
    values_need_delete.resize(cell_num);
    for(int i=0;i<cell_num;i++){
      Value value;
      row_tuple->cell_at(i,value);
      values_new.at(i)=value;
      values_need_delete.at(i)=value;
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

    RC rc = table_->make_record(static_cast<int>(values_new.size()), values_new.data(), record_new);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record. rc=%s", strrc(rc));
      return rc;
    } 

    rc = trx_->delete_record(table_, record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to delete record: %s", strrc(rc));
      return rc;
    }
    RecordFileScanner scanner;
    Record record_tmp;
    RowTuple tuple_origin ;


      TableMeta table_meta = table_->table_meta();
      int field_num=table_meta.field_num();
      std::vector<std::vector<int>> field_need_to_cmp_list_list;
      if(table_meta.index_num()==0){ //不进表扫描
        goto insert_it;
      } 
      rc = table_->get_record_scanner(scanner, trx_, true/*readonly*/);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create scanner . table=%s,, rc=%s", name(), strrc(rc));
        return rc;
      }



      tuple_origin.set_schema(table_, table_->table_meta().field_metas());
      tuple_origin.set_record(&record_new);


      for(int i=0;i<table_meta.index_num();i++){
        //第i个索引，该索引可以包括多个col
        
        std::vector<int> field_need_to_cmp_list;
        std::vector<std::string> index_field_name_list = table_meta.index(i)->fields();//在哪几个列建立了索引
        if(table_meta.index(i)->is_unique()==false){
          continue;//对于非unique的索引，不需要去重
        }
        for(int j=0;j<field_num;j++){
          for(int jj=0;jj<index_field_name_list.size();jj++){
            if(strcmp(table_meta.field(j)->name(),index_field_name_list[jj].c_str())==0){
              //需要建立unique索引的列，全部弹入list
              Value value1;
              tuple_origin.cell_at(j,value1);
              if(value1.is_null_value()) goto insert_it;//如果是null，不需要去重
              field_need_to_cmp_list.push_back(j);
              break;
            }
          }
        }
        if(field_need_to_cmp_list.size()>0) field_need_to_cmp_list_list.push_back(field_need_to_cmp_list);
      }
      if(field_need_to_cmp_list_list.size()==0) goto insert_it;//未建立unique索引，不需要去重



      while (scanner.has_next()) {
        rc = scanner.next(record_tmp);

        RowTuple tuple_tmp;
        tuple_tmp.set_schema(table_, table_->table_meta().field_metas());
        tuple_tmp.set_record(&record_tmp);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to scan records . table=%s,, rc=%s",name(), strrc(rc));
          return rc;
        }

        for(std::vector<int> field_need_to_cmp_list:field_need_to_cmp_list_list){
          bool flag=true;
          for(int field_need_to_cmp:field_need_to_cmp_list){
            Value value1;
            Value value2;
            tuple_tmp.cell_at(field_need_to_cmp,value1);
            tuple_origin.cell_at(field_need_to_cmp,value2);
            if(value1.compare(value2)!=0){
              flag=false;
              break;
            }
          }
          if(flag){//全部相等,跳过插入,update failure,回滚
            goto rollback;
          }
        }//比较结束
      }//扫描结束
      scanner.close_scan();

insert_it:
    rc = trx_->insert_record(table_, record_new);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
    }else{
      LOG_DEBUG("update success: insert new record success. rc=%s", strrc(rc));
      continue;
    }


   
rollback:
    LOG_WARN("update fail: duplicate key ,failed to insert record: %s", strrc(rc));
    RC rc2 = table_->make_record(static_cast<int>(values_need_delete.size()),values_need_delete.data(),record_need_delete);
    if(rc2!=RC::SUCCESS)
    {
      LOG_WARN("update rollback :failed to make deteled record. rc=%s", strrc(rc2));
      return rc2;
    }
    RC rc3 = trx_->insert_record(table_, record_need_delete);
    if(rc3!=RC::SUCCESS)
    {
      LOG_WARN("update rollback :failed to insert deteled record. rc=%s", strrc(rc2));
      return rc3;
    }
    LOG_WARN("update rollback :SUCCESS,Deleted record insert success. rc=%s", strrc(rc));
    return RC::RECORD_DUPLICATE_KEY;

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
