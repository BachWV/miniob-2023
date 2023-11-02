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
#include "sql/view/view.h"
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

static void wildcard_fields(const TableSrc &table_src, std::vector<AttrInfoSqlNode>& column_metas)
{
  if (auto p_table = std::get_if<Table*>(&table_src))
  {
    const Table *table = *p_table;
    const TableMeta &table_meta = table->table_meta();
    const int field_num = table_meta.field_num();
    for (int i = table_meta.sys_field_num(); i < field_num; i++) {
      const FieldMeta *field_meta = table_meta.field(i);
      AttrInfoSqlNode attr_info = {.type = field_meta->type(), .name = field_meta->name(), 
        .length = static_cast<size_t>(field_meta->len()), .nullable = field_meta->nullable()};
      column_metas.emplace_back(std::move(attr_info));
    }
  }
  else if (auto resolved_view = std::get_if<ResolvedView>(&table_src))
  {
    const std::vector<ViewColumnAttr> &col_name_infos = resolved_view->view_->view_columns();
    SelectStmt *view_select = resolved_view->select_.get();
    const std::vector<AttrInfoSqlNode> &col_attrs = view_select->column_attrs();

    size_t col_num = col_name_infos.size();
    for (size_t i = 0; i < col_num; ++i) {
      AttrInfoSqlNode attr_info = {.type = col_attrs[i].type, .name = col_name_infos[i].column_name, 
        .length = col_attrs[i].length, .nullable = col_attrs[i].nullable};
      column_metas.emplace_back(std::move(attr_info));
    }
  }
}

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

static std::string gen_output_name_for_field_id(bool need_table_name, const std::string &table_name, const std::string &field_name,
  const std::string &alias)
{
  if (!alias.empty())
    return alias;
  if (need_table_name)
    return table_name + "." + field_name;
  else
    return field_name;
}

RC SelectStmt::create(Db *db, ExprResolveContext *glob_ctx, const SelectSqlNode &select_sql, Stmt *&stmt, 
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> *correlate_exprs)
{
  auto convert_to_conjunction_type = [](Conditions::ConjunctionType type) {
    return type == Conditions::ConjunctionType::AND ? ConjunctionExpr::Type::AND : ConjunctionExpr::Type::OR;
  };

  auto gen_column_attr_info = [](const std::string &name, const ExprValueAttr &attr) {
    AttrInfoSqlNode attr_info;
    attr_info.name = name;
    attr_info.type = attr.type;
    attr_info.length = attr.length;
    attr_info.nullable = attr.nullable;
    return attr_info;
  };

  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  StmtResolveContext current_expr_ctx;  // 解析语句内表达式的上下文
  StmtResolveContext having_resolve_ctx;  // 解析having子句的上下文

  // collect tables in `from` statement
  std::unordered_map<std::string, TableSrc> table_map;  // table或view的名字（有别名则是别名，否则是表原名），映射到TableSrc
  std::vector<std::string> tables_;  // 按from的顺序排列的表名

  for (size_t i = 0; i < select_sql.relations.size(); i++) {
    const char *table_name = select_sql.relations[i].first.c_str();
    const std::string &alias_name = select_sql.relations[i].second;
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }
    std::string avail_table_name = alias_name.empty() ? table_name : alias_name;
    if (table_map.count(avail_table_name))
      return RC::SCHEMA_TABLE_EXIST;

    Table *table = db->find_table(table_name);
    if (nullptr != table) 
    {
      table_map.emplace(avail_table_name, table);
      tables_.emplace_back(avail_table_name);

      current_expr_ctx.add_table_to_namespace(avail_table_name, table);
      having_resolve_ctx.add_table_to_namespace(avail_table_name, table);
    }

    else  // 表为空，再查找视图，如果视图也找不到，就报错
    {
      View *view = db->find_view(table_name);
      if (view == nullptr)
      {
        LOG_WARN("no such table or view. db=%s, table_name=%s", db->name(), table_name);
        return RC::SCHEMA_TABLE_NOT_EXIST;
      }

      std::unique_ptr<SelectStmt> view_select;
      RC rc = view->resolve_select(db, glob_ctx, view_select);
      if (rc != RC::SUCCESS)
      {
        LOG_WARN("resolve view in from clause failed. rc=%s", strrc(rc));
        return rc;
      }

      /* 将view的元数据加入namespace */
      std::vector<SelectColumnInfo> &view_col_infos = view_select->select_expr_fields();
      std::vector<AttrInfoSqlNode> &view_col_value_attrs = view_select->column_attrs();
      const std::vector<ViewColumnAttr> &view_col_name_attrs = view->view_columns();
      size_t col_num = view_col_infos.size();
      assert(view_col_value_attrs.size() == col_num);
      assert(view_col_name_attrs.size() == col_num);

      for (size_t i = 0; i < col_num; ++i)
      {
        const AttrInfoSqlNode &col_value_attr = view_col_value_attrs[i];
        const FieldIdentifier &col_id = view_col_infos[i].tuple_cell_spec_;
        const ExprValueAttr col_attr = {.type = col_value_attr.type, .length = col_value_attr.length, 
          .nullable = col_value_attr.nullable};
        current_expr_ctx.add_view_column(avail_table_name, view_col_name_attrs.at(i).column_name, col_id, col_attr);
        having_resolve_ctx.add_view_column(avail_table_name, view_col_name_attrs.at(i).column_name, col_id, col_attr);
      }

      /* 将视图结构加入table_map_ */
      table_map.emplace(avail_table_name, ResolvedView(view, std::move(view_select)));
      tables_.emplace_back(avail_table_name);
    }
  }

  // collect fields in `group by` statement
  // std::vector<Field> group_fields;
  std::vector<FieldIdentifier> group_fields;
  for(const auto& rel_attr: select_sql.group_by_attrs){
    std::optional<FieldIdentifier> group_by_field;
    RC rc = current_expr_ctx.resolve_table_field_identifier(rel_attr.relation_name, rel_attr.attribute_name, group_by_field);
    if(rc != RC::SUCCESS || !group_by_field.has_value())
      return RC::SCHEMA_FIELD_NOT_EXIST;
    group_fields.emplace_back(std::move(group_by_field.value()));
  }

  // collect select_expr in `select` statement
  std::vector<SelectColumnInfo> select_columns;
  std::vector<AttrInfoSqlNode> column_attrs;  // 列的值属性
  std::vector<std::pair<bool, std::string>> simple_field_ident_infos;  // 列的field引用信息
  std::vector<AggExprInfo> all_agg_infos;  // select expr和having中的所有聚集函数
  bool has_agg = false;
  std::unordered_multiset<FieldIdentifier, FieldIdentifierHash> common_fields_set;  // 非agg的字段

  for (auto &select_expr_with_alias: select_sql.select_exprs)
  {
    const std::unique_ptr<ExprSqlNode> &select_expr = select_expr_with_alias.expr_;
    const std::string &expr_alias = select_expr_with_alias.alias_;

    // 如果表达式包含'*'
    if (is_star_identifier(select_expr.get()))
    {
      if (!expr_alias.empty()) 
        return RC::SCHEMA_FIELD_AMBIGUOUS;
        
      IdentifierExprSqlNode *id = static_cast<IdentifierExprSqlNode *>(select_expr.get());

      if (common::is_blank(id->get_table_name().c_str())) 
      {
        // 空.*
        for (auto &table_name : tables_) 
        {
          const TableSrc &table_src = table_map.at(table_name);
          std::vector<AttrInfoSqlNode> col_metas;
          wildcard_fields(table_src, col_metas);
          for (size_t i = 0; i < col_metas.size(); ++i)
          {
            const AttrInfoSqlNode &col_meta = col_metas[i];
            const std::string &col_name = col_meta.name;
            SelectColumnInfo col_info;
            col_info.tuple_cell_spec_ = glob_ctx->get_col_id_generator().generate_identifier();
            col_info.output_name_ = gen_output_name_for_field_id(table_map.size() > 1, table_name, col_name, expr_alias);
            ExprValueAttr col_attr = {.type = col_meta.type, .length = col_meta.length, .nullable = col_meta.nullable};

            std::unique_ptr<IdentifierExpr> id_expr;
            if (auto table = std::get_if<Table*>(&table_src))  // 如果是表，直接为该列建立到表列引用的FID
              id_expr = std::make_unique<IdentifierExpr>(FieldIdentifier(table_name, col_name));
            else {  // 否则是视图，需要拿到子查询中，能够引用视图该列的FID
              const ResolvedView &resolved_view = std::get<ResolvedView>(table_src);
              const FieldIdentifier &col_ident = resolved_view.select_->select_expr_fields()[i].tuple_cell_spec_;
              id_expr = std::make_unique<IdentifierExpr>(col_ident);
            }
            id_expr->set_value_attr(col_attr);
            id_expr->set_referred_table(table_name);
            col_info.expr_ = std::move(id_expr);

            select_columns.emplace_back(std::move(col_info));
            column_attrs.emplace_back(gen_column_attr_info(col_name, col_attr));
            simple_field_ident_infos.emplace_back(true, col_name);
            common_fields_set.emplace(table_name, col_name);  /* 生成一个表名.列名，用于group by的校验 */
          }
        }
      } 
      else
      {
        // 表名非空
        const char *table_name = id->get_table_name().c_str();
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

          TableSrc &table_src = iter->second;
          std::vector<AttrInfoSqlNode> col_metas;
          wildcard_fields(table_src, col_metas);
          for (size_t i = 0; i < col_metas.size(); ++i)
          {
            const AttrInfoSqlNode &col_meta = col_metas[i];
            const std::string &col_name = col_meta.name;
            SelectColumnInfo col_info;
            col_info.tuple_cell_spec_ = glob_ctx->get_col_id_generator().generate_identifier();
            col_info.output_name_ = gen_output_name_for_field_id(true, table_name, col_name, expr_alias);
            ExprValueAttr col_attr = {.type = col_meta.type, .length = col_meta.length, .nullable = col_meta.nullable};

            std::unique_ptr<IdentifierExpr> id_expr;
            if (auto table = std::get_if<Table*>(&table_src))  // 如果是表，直接为该列建立到表列引用的FID
              id_expr = std::make_unique<IdentifierExpr>(FieldIdentifier(table_name, col_name));
            else {  // 否则是视图，需要拿到子查询中，能够引用视图该列的FID
              const ResolvedView &resolved_view = std::get<ResolvedView>(table_src);
              const FieldIdentifier &col_ident = resolved_view.select_->select_expr_fields()[i].tuple_cell_spec_;
              id_expr = std::make_unique<IdentifierExpr>(col_ident);
            }
            id_expr->set_value_attr(col_attr);
            id_expr->set_referred_table(table_name);
            col_info.expr_ = std::move(id_expr);

            select_columns.emplace_back(std::move(col_info));
            column_attrs.emplace_back(gen_column_attr_info(col_name, col_attr));
            simple_field_ident_infos.emplace_back(true, col_name);
            common_fields_set.emplace(table_name, col_name);  /* 生成一个表名.列名，用于group by的校验 */
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
      col_info.tuple_cell_spec_ = glob_ctx->get_col_id_generator().generate_identifier();
      if (select_expr->get_type() == ExprSqlNodeType::Identifier)
      {
        auto *id = static_cast<IdentifierExprSqlNode *>(select_expr.get());
        
        std::string table_name = id->get_table_name();

        // 如果用户没有指定表名，则需要再resolve一遍得到table_name。原因：加了视图后，不能直接通过IdentifierExpr获得table_name了，
        // 因为从视图里获取的IdentifierExpr，它内部的FieldIdentifier是引用视图子查询列表达式的临时ID
        // 且IdentifierExprSqlNode中有可能不存在table_name(SQL里只有列名)
        // 这里不考虑列中的相关表达式，直接在当前作用域解析
        if (table_name.empty()) {
          rc = current_expr_ctx.resolve_field_referred_table(id->get_field_name(), table_name);
          HANDLE_RC(rc);
        }

        const std::string &field_name = id->get_field_name();
        col_info.output_name_ = gen_output_name_for_field_id(table_map.size() > 1, table_name, field_name, expr_alias);
        common_fields_set.emplace(table_name, field_name);  // 用于group by校验

        const std::string &column_outer_name = expr_alias.empty() ? field_name : expr_alias;
        column_attrs.emplace_back(gen_column_attr_info(column_outer_name, col_info.expr_->value_attr()));
        simple_field_ident_infos.emplace_back(true, field_name);
      }
      else
      {
        col_info.output_name_ = expr_alias.empty() ? select_expr->expr_name() : expr_alias;
        column_attrs.emplace_back(gen_column_attr_info(col_info.output_name_, col_info.expr_->value_attr()));
        simple_field_ident_infos.emplace_back(false, "");
      }

      std::vector<AggExprInfo> &agg_in_expr = select_expr_resolve_result.get_agg_expr_infos();
      if (!agg_in_expr.empty())
        has_agg = true;
      all_agg_infos.insert(all_agg_infos.end(), std::make_move_iterator(agg_in_expr.begin()), 
        std::make_move_iterator(agg_in_expr.end()));

      select_columns.emplace_back(std::move(col_info));
    }
  }

  // check aggregation validate: 如果存在agg，那么非agg的字段（common_fields_set）一定在gb中出现
  if(has_agg){
    // 非agg的字段和group by字段数一定相等，要不都是0。
    // 注：此处注释掉，不一定相等！group by中可以出现更多字段。非agg字段是group by字段的子集。
    if(common_fields_set.size() > group_fields.size()){
      return RC::SQL_SYNTAX;
    }

    // 若gf的字段在cfs中不存在，那么一定有错
    for(auto const& fi: group_fields){
      if(common_fields_set.count(fi) == 0){
        return RC::SQL_SYNTAX;
      }
    }
  }

  RC rc = RC::SUCCESS;

  std::vector<std::unique_ptr<Expression>> having_exprs;
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
    auto &agg_infos_in_having = having_resolve_result.get_agg_expr_infos();
    has_agg = agg_infos_in_having.empty() ? has_agg : true;
    all_agg_infos.insert(all_agg_infos.end(), std::make_move_iterator(agg_infos_in_having.begin()), 
      std::make_move_iterator(agg_infos_in_having.end()));
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
    std::optional<FieldIdentifier> field;
    rc = current_expr_ctx.resolve_table_field_identifier(orderby_sql_node.attr.relation_name, orderby_sql_node.attr.attribute_name,
      field);
    if (rc != RC::SUCCESS || !field.has_value())
      return RC::SCHEMA_FIELD_NOT_EXIST;
    order_fields.emplace_back(field.value(), orderby_sql_node.is_asc);
  }

  // everything alright
  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->table_map_.swap(table_map);
  select_stmt->select_expr_fields_.swap(select_columns);
  select_stmt->column_attrs_.swap(column_attrs);
  select_stmt->simple_field_ident_infos_.swap(simple_field_ident_infos);
  select_stmt->group_by_field_.swap(group_fields);
  select_stmt->order_fields_ = order_fields;
  select_stmt->has_where_ = has_where;
  select_stmt->where_exprs_ = std::make_unique<ConjunctionExpr>(convert_to_conjunction_type(select_sql.conditions.type), 
    where_exprs);
  select_stmt->sub_querys_in_where_.swap(apply_stmts);
  select_stmt->having_exprs_ = std::make_unique<ConjunctionExpr>(convert_to_conjunction_type(select_sql.having_attrs.type), 
    having_exprs);
  select_stmt->all_agg_infos_.swap(all_agg_infos);
  select_stmt->has_having_ = has_having;
  select_stmt->has_agg_ = has_agg;
  stmt = select_stmt;
  return RC::SUCCESS;
}
