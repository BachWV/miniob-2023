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
// Created by Wangyunlai on 2022/6/5.
//

#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "common/rc.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"

class FieldMeta;
class FilterStmt;
class Db;
class Table;
class Expression;
class CorrelateExpr;
class ExprResolveContext;
class ApplyStmt;

// 这里不应该叫“field”，应该是xx_stmt。但是我看按照他的方式，开stmt也不太合适，
// 我们只是需要个中间态的数据结构暂时保存下数据，索性就把数据放在Field这个文件里了
using SelectExprField = std::variant<Field, FieldsWithGroupBy>;

/**
 * @brief 表示select语句
 * @ingroup Statement
 */
class SelectStmt : public Stmt 
{
public:
  SelectStmt() = default;
  ~SelectStmt() = default;

  StmtType type() const override
  {
    return StmtType::SELECT;
  }

public:
  // glob_ctx: 用于解析表达式的全局环境
  // correlate_exprs：create中，需要构造本查询里引用外层查询的相关表达式，把层数和表达式包装为map
  static RC create(Db *db, ExprResolveContext *glob_ctx, const SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs);

public:
  const std::vector<Table *> &tables() const
  {
    return tables_;
  }
  const std::vector<SelectExprField> &select_expr_fields() const
  {
    return select_expr_fields_;
  }
  std::vector<std::unique_ptr<Expression>> &fetch_where_exprs() {
    return where_exprs_;
  }
  const std::vector<FieldWithOrder>& order_fields() const{
    return order_fields_;
  }
  const std::vector<Field> get_group_by_fields() const{
    return group_by_field_;
  }

private:
  RC resolve_select_expr_sql_node(const SelectExprSqlNode& sesn, SelectExprField& sef);

private:
  std::vector<Table *> tables_;
  std::vector<FieldWithOrder> order_fields_;
  
  std::vector<std::unique_ptr<Expression>> where_exprs_;  // 多个以AND连接的表达式
  std::vector<std::unique_ptr<ApplyStmt>> sub_querys_in_where_;  // where子句中的子查询
  std::vector<SelectExprField> select_expr_fields_;
  std::vector<Field> group_by_field_;
  std::vector<Field> resolved_query_field_;
};
