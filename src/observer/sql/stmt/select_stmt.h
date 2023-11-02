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
#include "sql/view/view.h"

class FieldMeta;
class FilterStmt;
class Db;
class Table;
class Expression;
class ConjunctionExpr;
class CorrelateExpr;
class ExprResolveContext;
class ApplyStmt;

struct SelectColumnInfo
{
  std::unique_ptr<Expression> expr_;  // 一个列就是一个表达式

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
  static RC create(Db *db, ExprResolveContext *glob_ctx, const SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs);

public:
  std::unordered_map<std::string, TableSrc> &table_map()
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
  std::vector<AggExprInfo> &fetch_all_agg_infos() { return all_agg_infos_; }
  std::vector<AttrInfoSqlNode> &column_attrs(){ return column_attrs_; }
  const std::vector<std::pair<bool, std::string>> &is_simple_field_ident_vec() const { return simple_field_ident_infos_; }

private:
  // 查询的表的名字（有别名则是别名，否则是原名），映射到对应的TableSrc对象
  // 多个表名可能会映射到同一个TableSrc对象，比如select * from table1 as t1, table1 as t2，
  // 每一个表名都对应一个算子树，所以用此结构代替原来的table_数组 
  std::unordered_map<std::string, TableSrc> table_map_;

  std::vector<FieldWithOrder> order_fields_;
  
  bool has_where_ = false;
  bool has_agg_ = false;
  std::unique_ptr<ConjunctionExpr> where_exprs_;
  std::vector<std::unique_ptr<ApplyStmt>> sub_querys_in_where_;

  std::vector<FieldIdentifier> group_by_field_;

  bool has_having_ = false;
  std::unique_ptr<ConjunctionExpr> having_exprs_;
  
  std::vector<AggExprInfo> all_agg_infos_;  // select expr和having中的所有聚集函数


  /* 
   * 以下的数据结构描述select的列的信息 
   * 出于历史原因（懂的都懂），数据结构分开成了多个vector维护
   */

  std::vector<SelectColumnInfo> select_expr_fields_;

  /*
   * 列的值属性(不同于SelectColumnInfo，这里主要描述这个列的值类型、大小、名字)
   * 其中name为列本身的名字。
   * 如果该列就是对表中field的引用，那么name就是该field的名字(就算是多表查询，name中也不带表名前缀，就是列本身的名字)或别名
   * 否则，列是其它表达式，则为表达式名或别名
   */
  std::vector<AttrInfoSqlNode> column_attrs_;

  /*
   * 使用select创建视图时需要此信息(对视图做更新时，如果更新被允许，则需要拿到视图的列所引用的基表名和列原名，所以需要解析select时传出这个信息)
   * 
   * pair的first表示：select中的每一个列，是不是到field的简单引用
   * 如果是(first为true)，则second为field的原名(不是别名)，且它引用的field所属表，可以从对应SelectColumnInfo.expr_中获取
   * 如果不是(first为false)，则second无效
   * 
   */
  std::vector<std::pair<bool, std::string>> simple_field_ident_infos_;
};
