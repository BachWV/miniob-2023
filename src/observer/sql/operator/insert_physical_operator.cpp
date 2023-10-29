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
  // TODO: 看孩子是否为空进行判断，有孩子，则孩子的分支直接打开下层算子
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
  //make_record结束
  for(auto &record : records){

    TableMeta table_meta = table_->table_meta();
    int field_num = table_meta.field_num();
    std::vector<std::vector<int>> unique_field_list_list;
    bool has_unique_index = table_meta.has_unique_index();
    bool need_to_scan = has_unique_index;
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
      rc = table_->get_record_scanner(scanner, trx, true/*readonly*/);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create scanner . table=%s,, rc=%s", name(), strrc(rc));
        return rc;
      }
      while (scanner.has_next()) {
        rc = scanner.next(record_tmp);
        RowTuple tuple_origin ;
        RowTuple tuple_tmp;
        tuple_origin.set_schema(table_, table_meta.field_metas());
        tuple_tmp.set_schema(table_, table_meta.field_metas());
        tuple_tmp.set_record(&record_tmp);
        tuple_origin.set_record(&record);
        if (rc != RC::SUCCESS) {
          LOG_WARN("failed to scan records . table=%s,, rc=%s",name(), strrc(rc));
          return rc;
        }

        for(std::vector<int> unique_field_list : unique_field_list_list){
          bool flag = true;
          for(int unique_field : unique_field_list){
            Value value1;
            Value value2;
            tuple_tmp.cell_at(unique_field,value1);
            tuple_origin.cell_at(unique_field,value2);
            if(value1.compare(value2)!=0){
              flag=false;
              break;
            }
          }
          if(flag){//全部相等,跳过插入,之后record也跳过
            LOG_DEBUG("skip insert record by transaction. rc=%s", strrc(rc));
            return RC::RECORD_DUPLICATE_KEY;
          }
        }//双层for结束
      }
      scanner.close_scan();
    }

    //结束判断，没有重复，插入
    rc = trx->insert_record(table_, record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
      return rc;
    }
      
  } //完成该record的插入

  return rc;
}

RC InsertPhysicalOperator::next()
{
    // TODO: 看孩子是否为空进行判断，若有孩子，那么具体的插入在这里
  return RC::RECORD_EOF;
}

RC InsertPhysicalOperator::close()
{
  return RC::SUCCESS;
}
