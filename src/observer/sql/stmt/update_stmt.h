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
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "common/rc.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/filter_stmt.h"

class Table;
class Db;
class ConjunctionExpr;
class Expression;
class ApplyStmt;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt 
{
public:
  UpdateStmt() = default;
  UpdateStmt(Table *table, std::unordered_map<int, std::unique_ptr<Expression>> &&value_list, 
    std::vector<std::unique_ptr<ApplyStmt>> &&subquerys_in_set_stmts,
    std::unique_ptr<ConjunctionExpr> cond_exprs);
 
  std::unique_ptr<ConjunctionExpr> fetch_cond_exprs();
  std::vector<std::unique_ptr<ApplyStmt>> &fetch_subquerys_in_set_stmts() { return subquerys_in_set_stmts_; }

  StmtType type() const override
  {
    return StmtType::UPDATE;
  }

 
public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table *table() const
  {
    return table_;
  }
  std::unordered_map<int, std::unique_ptr<Expression>> &fetch_value_list();

private:
  Table *table_ = nullptr;

  // key: 待修改的列在RowTuple中的下标，value: 表达式的值为修改的新值
  std::unordered_map<int, std::unique_ptr<Expression>> value_list_;
  std::vector<std::unique_ptr<ApplyStmt>> subquerys_in_set_stmts_;  // set表达式中的子查询
  std::unique_ptr<ConjunctionExpr> cond_exprs_;
};
