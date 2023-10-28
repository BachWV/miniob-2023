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
#include "sql/stmt/field_with_alias.h"
#include "sql/stmt/field_with_cul.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"
#include "sql/stmt/field_with_function.h"
#include "sql/expr/parsed_expr.h"

class FieldMeta;
class FilterStmt;
class Db;
class Table;
class Expression;
class ConjunctionExpr;
class CorrelateExpr;
class ExprResolveContext;
class ApplyStmt;

// 这里不应该叫“field”，应该是xx_stmt。但是我看按照他的方式，开stmt也不太合适，
// 我们只是需要个中间态的数据结构暂时保存下数据，索性就把数据放在Field这个文件里了
using SelectExprField = std::variant<FieldsWithGroupBy, FieldWithFunction, FieldWithCul, FieldWithAlias>;

struct SelectColumnInfo
{
  std::unique_ptr<Expression> expr_;  // 一个列就是一个表达式
  std::vector<AggExprInfo> agg_infos_;  // 该表达式中的所有聚集函数

  // 该列的虚拟列名对应的identifier，应该同时被添加到Proj和计算此表达式的FieldCulPhy中
  // 由于FieldCulPhy使用FieldMeta表示新字段的名字，需使用tuple_cell_spec_.field_name()初始化FieldMeta的name字段
  FieldIdentifier tuple_cell_spec_;

  std::string output_name_;  // 该列用于输出的名字
};

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
  static RC create(Db *db, ExprResolveContext *glob_ctx, SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs);

public:
  const std::unordered_map<std::string, Table *> &table_map() const
  {
    return table_map_;
  }
  std::vector<SelectColumnInfo> &select_expr_fields()
  {
    return select_expr_fields_;
  }
  bool has_where_clause() const { return has_where_; }
  std::unique_ptr<ConjunctionExpr> fetch_where_exprs();
  std::vector<std::unique_ptr<ApplyStmt>> &fetch_sub_querys_in_where() {
    return sub_querys_in_where_;
  }
  const std::vector<FieldWithOrder>& order_fields() const{
    return order_fields_;
  }
  const std::vector<FieldIdentifier> get_group_by_fields() const{
    return group_by_field_;
  }
  bool has_having_clause() const { return has_having_; }
  bool has_agg() const {return has_agg_;}
  std::unique_ptr<ConjunctionExpr> fetch_having_exprs();
  std::vector<AggExprInfo> &fetch_having_agg_infos() { return having_agg_infos_; }
  std::vector<AttrInfoSqlNode> column_attrs(){
    return column_attrs_;
  }

private:
  RC resolve_select_expr_sql_node(const SelectExprSqlNode& sesn, SelectExprField& sef);

private:
  // 查询的表的名字（有别名则是别名，否则是原名），映射到对应的Table对象
  // 多个表名可能会映射到同一个Table对象，比如select * from table1 as t1, table1 as t2，
  // 每一个表名都要建一个TableGet，所以用此结构代替原来的table_数组 
  std::unordered_map<std::string, Table *> table_map_;

  std::vector<FieldWithOrder> order_fields_;
  
  bool has_where_ = false;
  bool has_agg_ = false;
  std::unique_ptr<ConjunctionExpr> where_exprs_;
  std::vector<std::unique_ptr<ApplyStmt>> sub_querys_in_where_;

  std::vector<SelectColumnInfo> select_expr_fields_;
  std::vector<FieldIdentifier> group_by_field_;

  bool has_having_ = false;
  std::unique_ptr<ConjunctionExpr> having_exprs_;
  std::vector<AggExprInfo> having_agg_infos_;  // having中的聚集函数

  // select t1.id new_id, t2.name new_name, t1.score + t2.score as total_score from t1, t2 where t1.id = t2.id;
  std::vector<AttrInfoSqlNode> column_attrs_;  // 列的值属性
};
