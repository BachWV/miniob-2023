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
// Created by Wangyunlai on 2022/6/6.
//

#include <cassert>

#include "sql/stmt/field_with_alias.h"
#include "sql/stmt/select_stmt.h"
#include "sql/parser/parse_defs.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/field/field.h"
#include "storage/field/field_meta.h"
#include "storage/table/table.h"
#include "sql/expr/parsed_expr.h"
#include "sql/expr/expression.h"
#include "sql/stmt/apply_stmt.h"
#include <cstring>
#include <functional>
#include <unordered_set>

std::unique_ptr<ConjunctionExpr> SelectStmt::fetch_where_exprs()
{
  return std::move(where_exprs_);
}

std::unique_ptr<ConjunctionExpr> SelectStmt::fetch_having_exprs()
{
  return std::move(having_exprs_);
}

// 如果有表别名，则table_name里是别名，否则是表原名，内部不再使用从Table对象中拿到的名字
static void wildcard_fields(Table *table, const std::string &table_name, std::vector<std::string>& column_names)
{
  const TableMeta &table_meta = table->table_meta();
  const int field_num = table_meta.field_num();
  for (int i = table_meta.sys_field_num(); i < field_num; i++) {
    const FieldMeta *field_meta = table_meta.field(i);
    column_names.emplace_back(field_meta->name());
    // SelectColumnInfo column_info;
    // FieldIdentifier col_id(table_name, field_meta->name());
    // column_info.expr_ = std::make_unique<IdentifierExpr>(col_id);
    // column_info.col_id_ = col_id;
    // // 使用*的列不可能有列别名
    // column_infos.emplace_back(std::move(column_info));
  }
}

// @param order_fields[out]
// RC check_order_by_field(Db *db, const std::unordered_map<std::string, Table *>& table_map, const std::vector<Table *>& tables,
//   const std::vector<OrderByAttrSqlNode>& order_sql_nodes, std::vector<FieldWithOrder>& order_fields)
// {
//   for(auto& node : order_sql_nodes){
//     const RelAttrSqlNode &relation_attr = node.attr;

//     if (!common::is_blank(relation_attr.relation_name.c_str())) {
//       // 表名非空
//       const char *table_name = relation_attr.relation_name.c_str();
//       const char *field_name = relation_attr.attribute_name.c_str();
      
//       // 常规情况，table.field
//       auto iter = table_map.find(table_name);
//       if (iter == table_map.end()) {
//         LOG_WARN("no such table in from list: %s", table_name);
//         return RC::SCHEMA_FIELD_MISSING;
//       }
      
//       Table *table = iter->second;
//         const FieldMeta *field_meta = table->table_meta().field(field_name);
//         if (nullptr == field_meta) {
//           // 进行field合法性检查
//           LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
//           return RC::SCHEMA_FIELD_MISSING;
//         }
//         order_fields.push_back(FieldWithOrder(table, field_meta, node.is_asc));
//     } else {
//       // 没写table名，默认从第一个table中找
//       if (tables.size() != 1) {
//         LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name.c_str());
//         return RC::SCHEMA_FIELD_MISSING;
//       }

//       Table *table = tables[0];
//       const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name.c_str());
//       if (nullptr == field_meta) {
//         LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name.c_str());
//         return RC::SCHEMA_FIELD_MISSING;
//       }
//       order_fields.push_back(FieldWithOrder(table, field_meta, node.is_asc));
//     }
//   }
//   return RC::SUCCESS;
// }

// 只检查具名的字段(如id，name等)，带有"*"不会通过check。RelAttrNode -> Field
// 成功返回RC::success，并返回构建好的field；失败返回RC::SCHEMA_FIELD_MISSING
RC resolve_common_field(Db *db, const std::unordered_map<std::string, Table *>& table_map, 
  const RelAttrSqlNode& relation_attr, FieldIdentifier& field)
{
  if (!common::is_blank(relation_attr.relation_name.c_str())) {
    // 表名非空
    const char *table_name = relation_attr.relation_name.c_str();
    const char *field_name = relation_attr.attribute_name.c_str();
    
    // 常规情况，table.field
    auto iter = table_map.find(table_name);
    if (iter == table_map.end()) {
      LOG_WARN("no such table in from list: %s", table_name);
      return RC::SCHEMA_FIELD_MISSING;
    }
    
    Table *table = iter->second;
    const FieldMeta *field_meta = table->table_meta().field(field_name);
    if (nullptr == field_meta) {
      // 进行field合法性检查
      LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
      return RC::SCHEMA_FIELD_MISSING;
    }
    field = FieldIdentifier(table_name, field_name);
    return RC::SUCCESS;
  } 
  // 只有列名，则遍历所有的表
  else {
    const std::string &field_name = relation_attr.attribute_name;
    std::string target_table_name;
    const FieldMeta *target_field = nullptr;
    for (const auto &[ele_table_name, table]: table_map) 
    {
        const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
        if (field_meta != nullptr)
        {
            if (target_field != nullptr)  // 多个表都有field_name这个字段，返回解析失败，而不是找不到
            {
                LOG_WARN("field name %s is ambiguous", field_name.c_str());
                return RC::SCHEMA_FIELD_AMBIGUOUS;
            }
            target_table_name = ele_table_name;
            target_field = field_meta;
        }
    }
    if (target_field != nullptr) {
      field = FieldIdentifier(target_table_name, field_name);
      return RC::SUCCESS;
    }
    return RC::SCHEMA_FIELD_MISSING;
  }
}

static bool is_star_identifier(ExprSqlNode *expr)
{
  if (expr->get_type() == ExprSqlNodeType::Identifier)
  {
    IdentifierExprSqlNode *id = static_cast<IdentifierExprSqlNode *>(expr);
    if (id->get_field_name() == "*")
      return true;
  }
  return false;
}

static std::string gen_output_name_for_field_id(bool need_table_name, const std::string &table_name, const std::string &field_name)
{
  if (need_table_name)
    return table_name + "." + field_name;
  else
    return field_name;
}

RC SelectStmt::create(Db *db, ExprResolveContext *glob_ctx, SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs)
{
  auto convert_to_conjunction_type = [](Conditions::ConjunctionType type) {
    return type == Conditions::ConjunctionType::AND ? ConjunctionExpr::Type::AND : ConjunctionExpr::Type::OR;
  };

  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  StmtResolveContext current_expr_ctx;  // 解析语句内表达式的上下文
  StmtResolveContext having_resolve_ctx;  // 解析having子句的上下文

  // collect tables in `from` statement
  std::vector<Table *> tables;
  std::unordered_map<std::string, Table *> table_map;

  /* 做别名时记得在这里修改 */
  for (size_t i = 0; i < select_sql.relations.size(); i++) {
    const char *table_name = select_sql.relations[i].c_str();
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }

    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    tables.push_back(table);
    table_map.insert(std::pair<std::string, Table *>(table_name, table));

    current_expr_ctx.add_table_to_namespace(table_name, table);
    having_resolve_ctx.add_table_to_namespace(table_name, table);
  }

  // collect fields in `group by` statement
  // std::vector<Field> group_fields;
  std::vector<FieldIdentifier> group_fields;
  for(const auto& rel_attr: select_sql.group_by_attrs){
    FieldIdentifier group_by_field;
    auto rc = resolve_common_field(db, table_map, rel_attr, group_by_field);
    if(rc != RC::SUCCESS){
      return rc;
    }
    group_fields.emplace_back(std::move(group_by_field));
  }

  // collect select_expr in `select` statement
  std::vector<SelectColumnInfo> select_columns;
  bool has_agg = false;
  std::unordered_multiset<FieldIdentifier, FieldIdentifierHash> common_fields_set;  // 非agg的字段

  for (auto &select_expr_with_alias: select_sql.select_exprs)
  {
    std::unique_ptr<ExprSqlNode> &select_expr = select_expr_with_alias.expr_;
    const std::string &expr_alias = select_expr_with_alias.alias_;

    // 如果表达式包含'*'
    if (is_star_identifier(select_expr.get()))
    {
      IdentifierExprSqlNode *id = static_cast<IdentifierExprSqlNode *>(select_expr.get());

      if (common::is_blank(id->get_table_name().c_str())) 
      {
        // 空.*
        for (Table *table : tables) 
        {
          std::string table_name = table->name();  // 有表别名时需要在这改

          std::vector<std::string> col_names;
          wildcard_fields(table, table_name, col_names);
          for (auto &col_name: col_names)
          {
            SelectColumnInfo col_info;
            col_info.expr_ = std::make_unique<IdentifierExpr>(FieldIdentifier(table_name, col_name));
            col_info.tuple_cell_spec_ = glob_ctx->get_col_id_generator().generate_identifier();
            col_info.output_name_ = gen_output_name_for_field_id(table_map.size() > 1, table_name, col_name);
            select_columns.emplace_back(std::move(col_info));

            /* 生成一个表名.列名，用于group by的校验 */
            common_fields_set.emplace(table_name, col_name);
          }
        }
      } 
      else
      {
        // 表名非空
        const char *table_name = id->get_table_name().c_str();
        const char *field_name = id->get_field_name().c_str();
        
        if (0 == strcmp(table_name, "*")) {
          // *.*，不合法
          return RC::SCHEMA_FIELD_MISSING;
        } 
        
        // table.*
        else {
          auto iter = table_map.find(table_name);
          if (iter == table_map.end()) {
            LOG_WARN("no such table in from list: %s", table_name);
            return RC::SCHEMA_FIELD_MISSING;
          }

          Table *table = iter->second;
          std::vector<std::string> col_names;
          wildcard_fields(table, id->get_table_name(), col_names);
          for (auto &col_name: col_names)
          {
            SelectColumnInfo col_info;
            col_info.expr_ = std::make_unique<IdentifierExpr>(FieldIdentifier(table_name, col_name));
            col_info.tuple_cell_spec_ = glob_ctx->get_col_id_generator().generate_identifier();
            col_info.output_name_ = gen_output_name_for_field_id(true, table_name, col_name);
            select_columns.emplace_back(std::move(col_info));

            /* 生成一个表名.列名，用于group by的校验 */
            common_fields_set.emplace(table_name, col_name);
          }
        }
      }
    }

    // 表达式不包含'*'
    else
    {
      ExprResolveResult select_expr_resolve_result;
      glob_ctx->push_stmt_ctx(&current_expr_ctx);
      auto rc = select_expr->resolve(glob_ctx, &select_expr_resolve_result);
      HANDLE_RC(rc);
      glob_ctx->pop_stmt_ctx();

      // 目前不支持select列中的子查询，要支持在这改
      assert(select_expr_resolve_result.get_subquerys_in_expr().empty());
      assert(select_expr_resolve_result.get_correlate_exprs().empty());

      SelectColumnInfo col_info;
      col_info.expr_ = select_expr_resolve_result.owns_result_expr_tree();
      col_info.agg_infos_ = std::move(select_expr_resolve_result.get_agg_expr_infos());
      col_info.tuple_cell_spec_ = glob_ctx->get_col_id_generator().generate_identifier();
      if (select_expr->get_type() == ExprSqlNodeType::Identifier)
      {
        IdentifierExpr *id = static_cast<IdentifierExpr *>(col_info.expr_.get());
        const std::string &table_name = id->field().table_name();
        const std::string &field_name = id->field().field_name();
        col_info.output_name_ = gen_output_name_for_field_id(table_map.size() > 1, table_name, field_name);
        common_fields_set.emplace(table_name, field_name);  // 用于group by校验
      }
      else
        col_info.output_name_ = select_expr->expr_name();

      if (!col_info.agg_infos_.empty())
        has_agg = true;

      select_columns.emplace_back(std::move(col_info));
    }
  }

    // }else if(auto agg_sql_node = get_if<AggregateFuncSqlNode>(&select_expr_field)){
    //   has_agg = true;
    //   agg_functions_names.push_back(agg_sql_node->name);
    //   Field agg_field;
    //   auto rc = resolve_common_field(db, table_map, tables, agg_sql_node->attr, agg_field);
    //   if(rc != RC::SUCCESS){
    //     // 若为COUNT(*)的处理。AGG算子中要手动释放agg_field的内存
    //     if(agg_sql_node->agg_op == AGG_COUNT && 0 == strcmp(agg_sql_node->attr.attribute_name.c_str(), "*") ){
    //       FieldMeta* meta =  new FieldMeta("*", INTS, 1,1,false, false);
    //       assert(tables[0]);
    //       agg_field = Field(tables[0], meta);
    //     }else{
    //       return rc;
    //     }
    //   }
    //   bool with_table_name = tables.size() > 1;
    //   auto fwgb = FieldsWithGroupBy(agg_field, group_fields, agg_sql_node->agg_op, with_table_name, true);
    //   fwgb.alias_ = agg_sql_node->alias_;
    //   select_expr_fields.push_back(std::move(fwgb));
    // }

  // check aggregation validate: 如果存在agg，那么非agg的字段（common_fields_set）一定在gb中出现
  if(has_agg){
    // 非agg的字段和group by字段数一定相等，要不都是0。
    // 注：此处注释掉，不一定相等！group by中可以出现更多字段。非agg字段是group by字段的子集。
    // if(common_fields_set.size() != group_fields.size()){
    //   return RC::SQL_SYNTAX;
    // }

    // 若gf的字段在cfs中不存在，那么一定有错
    for(auto const& fi: group_fields){
      if(common_fields_set.count(fi) == 0){
        return RC::SQL_SYNTAX;
      }
    }
  }


  RC rc = RC::SUCCESS;
  /* 开始解析HAVING子句
   * 把Having里可能出现的列名，调用having_resolve_ctx.add_other_column_name传入，目前后续处理中没有考虑having的聚集是否出现在select列中
   * 这里先省略了
   */
  // for(auto& str: agg_functions_names){
  //   having_resolve_ctx.add_other_column_name(str);
  // }

  std::vector<std::unique_ptr<Expression>> having_exprs;
  std::vector<AggExprInfo> aggs_in_having;
  glob_ctx->push_stmt_ctx(&having_resolve_ctx);

  bool has_having = false;
  if (select_sql.having_attrs.exprs.size() > 0)
    has_having = true;

  for (auto &having_expr: select_sql.having_attrs.exprs)
  {
    ExprResolveResult having_resolve_result;
    rc = having_expr->resolve(glob_ctx, &having_resolve_result);
    if (rc != RC::SUCCESS)
    {
      LOG_WARN("resolve having expr failed. rc=%d", strrc(rc));
      return rc;
    }

    assert(having_resolve_result.get_subquerys_in_expr().empty());
    assert(having_resolve_result.get_correlate_exprs().empty());

    having_exprs.emplace_back(having_resolve_result.owns_result_expr_tree());

    // 将所有having中的聚集推到logic oper处理
    // 处理时不用再校验了，expr的resolve保证AggExprInfo里的东西是有效的
    aggs_in_having.swap(having_resolve_result.get_agg_expr_infos());

    // for(auto& agg_info: agg_infos){
    //   Field agg_field;
    //   RelAttrSqlNode attr;
    //   attr.relation_name = agg_info.agg_field().table_name();
    //   attr.attribute_name = agg_info.agg_field().field_name();
    //   auto rc = resolve_common_field(db, table_map, tables, attr, agg_field);
    //   if(rc != RC::SUCCESS){
    //     // 若为COUNT(*)的处理。AGG算子中要手动释放agg_field的内存
    //     if(agg_info.agg_op() == AGG_COUNT && 0 == strcmp(attr.attribute_name.c_str(), "*") ){
    //       FieldMeta* meta =  new FieldMeta("*", INTS, 1,1,false, false);
    //       assert(tables[0]);
    //       agg_field = Field(tables[0], meta);
    //     }else{
    //       return rc;
    //     }
    //   }
    //   bool with_table_name = tables.size() > 1;
    //   select_expr_fields.push_back(FieldsWithGroupBy(agg_field, group_fields, agg_info.agg_op(), with_table_name, false));
    // }
  }
  glob_ctx->pop_stmt_ctx();

  /* 开始解析where子句 */
  std::vector<std::unique_ptr<ApplyStmt>> apply_stmts;
  std::vector<std::unique_ptr<Expression>> where_exprs;
  glob_ctx->push_stmt_ctx(&current_expr_ctx);

  // 解析每一个where条件
  bool has_where = false;
  if (select_sql.conditions.exprs.size() > 0)
    has_where = true;

  for (auto &where_expr_sql_node: select_sql.conditions.exprs)
  {
    ExprResolveResult where_resolve_res;
    rc = where_expr_sql_node->resolve(glob_ctx, &where_resolve_res);
    if (rc != RC::SUCCESS)
    {
      LOG_WARN("resolve where expr failed. rc=%d", strrc(rc));
      return rc;
    }

    /* 拿到where中的子查询 */
    std::vector<std::unique_ptr<ApplyStmt>>& subquerys_in_where = where_resolve_res.get_subquerys_in_expr();
    for (auto &subquery: subquerys_in_where)
      apply_stmts.emplace_back(std::move(subquery));

    /* 
     * 处理where条件的相关表达式
     * where条件中如果出现了相关表达式，那么本查询也是一个相关子查询，把相关表达式添加到参数correlate_exprs中
     */
    std::unordered_map<size_t, std::vector<CorrelateExpr*>>& correlate_exprs_in_where = where_resolve_res.get_correlate_exprs();
    for (auto &[level, exprs]: correlate_exprs_in_where)
    {
      assert(level < glob_ctx->current_level());
      for (CorrelateExpr *expr: exprs)
        (*correlate_exprs)[level].push_back(expr);
    }

    where_exprs.emplace_back(where_resolve_res.owns_result_expr_tree());  // 记录生成的表达式树
  }
  
  glob_ctx->pop_stmt_ctx();

  // collect query fields in `order by` statement
  std::vector<FieldWithOrder> order_fields;
  for (auto &orderby_sql_node: select_sql.order_by_attrs)
  {
    FieldIdentifier field;
    rc = resolve_common_field(db, table_map, orderby_sql_node.attr, field);
    HANDLE_RC(rc);
    order_fields.emplace_back(field, orderby_sql_node.is_asc);
  }

  // rc = check_order_by_field(db, table_map, tables, select_sql.order_by_attrs, order_fields);
  // if(rc != RC::SUCCESS){
  //   // log在check_order_by_field中打过了
  //   return rc;
  // }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->tables_.swap(tables);
  select_stmt->select_expr_fields_.swap(select_columns);
  select_stmt->group_by_field_.swap(group_fields);
  select_stmt->order_fields_ = order_fields;
  select_stmt->has_where_ = has_where;
  select_stmt->where_exprs_ = std::make_unique<ConjunctionExpr>(convert_to_conjunction_type(select_sql.conditions.type), 
    where_exprs);
  select_stmt->sub_querys_in_where_.swap(apply_stmts);
  select_stmt->having_exprs_ = std::make_unique<ConjunctionExpr>(convert_to_conjunction_type(select_sql.having_attrs.type), 
    having_exprs);
  select_stmt->having_agg_infos_.swap(aggs_in_having);
  select_stmt->has_having_ = has_having;
  stmt = select_stmt;
  return RC::SUCCESS;
}
