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
class Expression;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt 
{
public:
  UpdateStmt() = default;
  UpdateStmt(Table *table,std::unordered_map<int,Value> value_list, std::vector<std::unique_ptr<Expression>> &&cond_exprs);
 
  std::vector<std::unique_ptr<Expression>>& fetch_cond_exprs()
  {
    return cond_exprs_;
  }

  StmtType type() const override
  {
    return StmtType::UPDATE;
  }

 
public:
  static RC create(Db *db, UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table *table() const
  {
    return table_;
  }
  const std::unordered_map<int, Value> &value_list() const
  {
    return value_list_;
  }

private:
  Table *table_ = nullptr;
  std::unordered_map<int, Value> value_list_;
  std::vector<std::unique_ptr<Expression>> cond_exprs_;
};
