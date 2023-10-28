/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by WangYunlai on 2021/6/9.
//

#include "sql/operator/insert_physical_operator.h"
#include "sql/stmt/insert_stmt.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

using namespace std;

InsertPhysicalOperator::InsertPhysicalOperator(Table *table, vector<vector<Value>> &&value_rows)
    : table_(table), value_rows_(std::move(value_rows))
{}

RC InsertPhysicalOperator::open(Trx *trx)
{
  RC rc= RC::SUCCESS;
  std::vector<Record> records;
  for(auto &values : value_rows_) {
    Record record;
    rc = table_->make_record(static_cast<int>(values.size()), values.data(), record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record. rc=%s", strrc(rc));
      return rc;
    }
    records.push_back(record);
  }
  for(auto &record : records){
      RecordFileScanner scanner;
      Record record_tmp;
      


      TableMeta table_meta = table_->table_meta();
      int field_num=table_meta.field_num();
      std::vector<std::vector<int>> field_need_to_cmp_list_list;
      if(table_meta.index_num()==0){ //不进表扫描
        goto insert_it;
      } 
      rc = table_->get_record_scanner(scanner, trx, true/*readonly*/);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create scanner . table=%s,, rc=%s", name(), strrc(rc));
        return rc;
      }


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
              field_need_to_cmp_list.push_back(j);
              break;
            }
          }//O(nm)的列名匹配
    
        }
          if(field_need_to_cmp_list.size()>0) field_need_to_cmp_list_list.push_back(field_need_to_cmp_list);
      }
      if(field_need_to_cmp_list_list.size()==0) goto insert_it;


      while (scanner.has_next()) {
        rc = scanner.next(record_tmp);
        RowTuple tuple_origin ;
        RowTuple tuple_tmp;
        tuple_origin.set_schema(table_, table_->table_meta().field_metas());
        tuple_tmp.set_schema(table_, table_->table_meta().field_metas());
        tuple_tmp.set_record(&record_tmp);
        tuple_origin.set_record(&record);
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
          if(flag){//全部相等,跳过插入
            goto skip_insert;
          }
        } 
      }
      scanner.close_scan();



insert_it:
    rc = trx->insert_record(table_, record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
      return rc;
    }
    break;
    

skip_insert:
    LOG_DEBUG("skip insert record by transaction. rc=%s", strrc(rc));
    return RC::RECORD_DUPLICATE_KEY;
  }
  
  return rc;
}

RC InsertPhysicalOperator::next()
{
  return RC::RECORD_EOF;
}

RC InsertPhysicalOperator::close()
{
  return RC::SUCCESS;
}
