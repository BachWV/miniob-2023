/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2023/08/16.
//

#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "common/rc.h"

class Stmt;
class CalcStmt;
class SelectStmt;
class FilterStmt;
class InsertStmt;
class DeleteStmt;
class UpdateStmt;
class ExplainStmt;
class LogicalOperator;
class Expression;
class ConjunctionExpr;
class CreateTableSelectStmt;

class LogicalPlanGenerator
{
public:
  LogicalPlanGenerator() = default;
  virtual ~LogicalPlanGenerator() = default;

  RC create(Stmt *stmt, std::unique_ptr<LogicalOperator> &logical_operator);

private:
  RC create_plan(CalcStmt *calc_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  std::unique_ptr<LogicalOperator> create_select_oper_tree(std::vector<std::unique_ptr<LogicalOperator>>&);
  RC create_plan(SelectStmt *select_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(FilterStmt *filter_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(InsertStmt *insert_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(DeleteStmt *delete_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(UpdateStmt *update_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(ExplainStmt *explain_stmt, std::unique_ptr<LogicalOperator> &logical_operator);
  RC create_plan(CreateTableSelectStmt *cts_stmt, std::unique_ptr<LogicalOperator> &logical_operator);

  // 返回的PredicateLogicOperator接管cond_exprs中Expression的所有权
  RC create_plan(std::unique_ptr<ConjunctionExpr> cond_exprs, std::unique_ptr<LogicalOperator> &logical_operator);
};