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
// Created by Longda on 2021/4/13.
//

#include <string>
#include <sstream>

#include "common/rc.h"
#include "sql/executor/execute_stage.h"

#include "common/log/log.h"
#include "session/session.h"
#include "event/storage_event.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "sql/operator/insert_multi_physical_operator.h"
#include "sql/operator/project_physical_operator.h"
#include "sql/stmt/create_table_select_stmt.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/select_stmt.h"
#include "storage/default/default_handler.h"
#include "sql/executor/command_executor.h"
#include "sql/operator/calc_physical_operator.h"

using namespace std;
using namespace common;

// 这个执行器的switch在command_executor中
RC ExecuteStage::handle_request(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;
  const unique_ptr<PhysicalOperator> &physical_operator = sql_event->physical_operator();
  if (physical_operator != nullptr) {
    return handle_request_with_physical_operator(sql_event);
  }

  SessionEvent *session_event = sql_event->session_event();

  Stmt *stmt = sql_event->stmt();
  if (stmt != nullptr) {
    CommandExecutor command_executor;
    rc = command_executor.execute(sql_event);
    session_event->sql_result()->set_return_code(rc);
  } else {
    return RC::INTERNAL;
  }
  return rc;
}

RC ExecuteStage::handle_request_with_physical_operator(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;

  // 所以我optimiser进行了这么多次逻辑和物理的转换，到最后现在还是离不开parser阶段的stmt
  // 转换的有点逆天吧
  Stmt *stmt = sql_event->stmt();
  ASSERT(stmt != nullptr, "SQL Statement shouldn't be empty!");

  unique_ptr<PhysicalOperator> &physical_operator = sql_event->physical_operator();
  ASSERT(physical_operator != nullptr, "physical operator should not be null");

  // TODO 这里也可以优化一下，是否可以让physical operator自己设置tuple schema
  TupleSchema schema;
  switch (stmt->type()) {
    case StmtType::SELECT: {
      SelectStmt *select_stmt = static_cast<SelectStmt *>(stmt);
      // // 用alias
      // for (const Field &field : select_stmt->query_fields()) {
      //   if (with_table_name) {
      //     schema.append_cell(field.table_name(), field.field_name());
      //   } else {
      //     schema.append_cell(field.field_name());
      //   }
      // }

      auto proj_oper = dynamic_cast<ProjectPhysicalOperator*>(physical_operator.get());
      proj_oper->set_schema(schema);
      
    } break;

    case StmtType::CALC: {
      CalcPhysicalOperator *calc_operator = static_cast<CalcPhysicalOperator *>(physical_operator.get());
      for (const unique_ptr<Expression> & expr : calc_operator->expressions()) {
        schema.append_cell(expr->name().c_str());
      }
    } break;

    case StmtType::EXPLAIN: {
      schema.append_cell("Query Plan");
    } break;

    case StmtType::CREATE_TABLE_SELECT:{
      CreateTableSelectStmt* cts_stmt = static_cast<CreateTableSelectStmt*>(stmt);
      // 1. 根据解析出来的proj和从db中拿到的字段名来create table
      Session *session = sql_event->session_event()->session();
      RC rc = session->get_current_db()->create_table(cts_stmt->created_table_name().c_str(), cts_stmt->created_table_infos().size(), cts_stmt->created_table_infos().data());
      HANDLE_RC(rc);
      // 2. static_cast
      auto insert_multi_oper = dynamic_cast<InsertMultiPhysicalOperator*>(physical_operator.get());
      // 3. insert_multy.set(table)
      insert_multi_oper->set_table(session->get_current_db()->find_table(cts_stmt->created_table_name().c_str()));
      // 4. 执行并判断是否成功
      Trx *trx = session->current_trx();
      trx->start_if_need();
      rc = insert_multi_oper->open(trx);
      HANDLE_RC(rc);

      while(RC::SUCCESS == (rc = insert_multi_oper->next())); // 一直执行完
      
      if(rc != RC::RECORD_EOF){
        sql_event->session_event()->sql_result()->set_return_code(rc);
        return rc;
      }

      rc = (rc == RC::RECORD_EOF) ? RC::SUCCESS : rc;
      sql_event->session_event()->sql_result()->set_return_code(rc);
      return rc;
    } break;
    default: {
      // 只有select返回结果
    } break;
  }

  SqlResult *sql_result = sql_event->session_event()->sql_result();
  sql_result->set_tuple_schema(schema);
  sql_result->set_operator(std::move(physical_operator));
  return rc;
}
