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

#include "sql/optimizer/logical_plan_generator.h"

#include "common/lang/string.h"
#include "sql/operator/aggregate_logical_operator.h"
#include "sql/operator/deduplicate_logical_operator.h"
#include "sql/operator/logical_operator.h"
#include "sql/operator/calc_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/update_logical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/explain_logical_operator.h"
#include "sql/operator/sort_logical_operator.h"
#include "sql/operator/function_logical_operator.h"
#include "sql/stmt/create_table_stmt.h"
#include "sql/stmt/field_with_function.h"
#include "sql/operator/field_cul_logical_operator.h"
#include "sql/operator/insert_multi_logical_operator.h"


#include "sql/stmt/stmt.h"
#include "sql/stmt/calc_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/explain_stmt.h"
#include "sql/stmt/apply_stmt.h"
#include "storage/field/field.h"
#include "sql/stmt/create_table_select_stmt.h"
#include <memory>

using namespace std;

RC LogicalPlanGenerator::create(Stmt *stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  RC rc = RC::SUCCESS;
  switch (stmt->type()) {
    case StmtType::CALC: {
      CalcStmt *calc_stmt = static_cast<CalcStmt *>(stmt);
      rc = create_plan(calc_stmt, logical_operator);
    } break;

    case StmtType::SELECT: {
      SelectStmt *select_stmt = static_cast<SelectStmt *>(stmt);
      rc = create_plan(select_stmt, logical_operator);
    } break;

    case StmtType::INSERT: {
      InsertStmt *insert_stmt = static_cast<InsertStmt *>(stmt);
      rc = create_plan(insert_stmt, logical_operator);
    } break;

    case StmtType::DELETE: {
      DeleteStmt *delete_stmt = static_cast<DeleteStmt *>(stmt);
      rc = create_plan(delete_stmt, logical_operator);
    } break;

    case StmtType::UPDATE: {
      UpdateStmt *update_stmt = static_cast<UpdateStmt *>(stmt);
      rc = create_plan(update_stmt, logical_operator);
    } break;

    case StmtType::EXPLAIN: {
      ExplainStmt *explain_stmt = static_cast<ExplainStmt *>(stmt);
      rc = create_plan(explain_stmt, logical_operator);
    } break;

    case StmtType::CREATE_TABLE_SELECT: {
      CreateTableSelectStmt* cts_stmt = static_cast<CreateTableSelectStmt *>(stmt);
      rc = create_plan(cts_stmt, logical_operator);
    } break;

    default: {
      rc = RC::UNIMPLENMENT;
    }
  }
  return rc;
}


RC LogicalPlanGenerator::create_plan(CalcStmt *calc_stmt, std::unique_ptr<LogicalOperator> &logical_operator)
{
  logical_operator.reset(new CalcLogicalOperator(std::move(calc_stmt->expressions())));
  return RC::SUCCESS;
}

// op1 -> op2 -> op3
// 没做为空的检查，调用者自己去保证正确性
std::unique_ptr<LogicalOperator> LogicalPlanGenerator::create_select_oper_tree(std::vector<std::unique_ptr<LogicalOperator>>& opers){
  auto curr = opers.begin();
  // 找到第一个不为null的oper
  while(curr != opers.end() && *curr == nullptr ){
    curr++;
  }
  auto next = curr+1;
  while(next != opers.end() && *next == nullptr){
    next++;
  }

  while(next != opers.end()){
    (*next)->add_child(std::move((*curr)));
    curr = next;
    
    do{
      next++;
    }while(next != opers.end() && *next == nullptr);
  }

  return std::move(*curr);
}

RC LogicalPlanGenerator::create_plan(
    SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  RC rc = RC::SUCCESS;

  unique_ptr<LogicalOperator> table_oper(nullptr);
  std::vector<std::unique_ptr<LogicalOperator>> opers;

  auto &table_map = select_stmt->table_map();
  std::vector<SelectColumnInfo>& select_expr_fields = select_stmt->select_expr_fields();

  // table-get / join
  for (auto& [table_name, table_src] : table_map) {  // 如果是单表，建一个table_get
    unique_ptr<LogicalOperator> cur_oper;
    if (auto table = std::get_if<Table*>(&table_src)) {
      cur_oper = std::make_unique<TableGetLogicalOperator>(*table, table_name, true/*readonly*/);
    }
    else {  // 如果是视图，创建视图的子查询算子树
      ResolvedView &resolved_view = std::get<ResolvedView>(table_src);
      rc = create_plan(resolved_view.select_.get(), cur_oper);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to create select logical plan for view. rc=%s", strrc(rc));
        return rc;
      }
    }

    if (table_oper == nullptr) {
      table_oper = std::move(cur_oper);
    } else {
      JoinLogicalOperator *join_oper = new JoinLogicalOperator;
      join_oper->add_child(std::move(table_oper));
      join_oper->add_child(std::move(cur_oper));
      table_oper = unique_ptr<LogicalOperator>(join_oper);
    }
  }
  opers.push_back(std::move(table_oper));

  // subquerys in where clause
  std::vector<std::unique_ptr<ApplyStmt>> &apply_stmts = select_stmt->fetch_sub_querys_in_where();
  for (std::unique_ptr<ApplyStmt> &apply_stmt : apply_stmts) {
    unique_ptr<LogicalOperator> apply_oper;
    RC rc = apply_stmt->create_logic_plan(apply_oper);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create apply logical plan. rc=%s", strrc(rc));
      return rc;
    }
    opers.push_back(std::move(apply_oper));
  }

  if (select_stmt->has_where_clause()) {
    unique_ptr<LogicalOperator> predicate_oper(nullptr);
    RC rc = create_plan(select_stmt->fetch_where_exprs(), predicate_oper);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create predicate logical plan. rc=%s", strrc(rc));
      return rc;
    }
    opers.push_back(std::move(predicate_oper));
  }

  // agg / field-cul-expr: 构建算子的同时把all_fields按序构建出来
  std::vector<FieldIdentifier> all_field_identifiers;
  std::vector<std::string> all_output_names;
  if(select_stmt->has_agg()){
    vector<FieldWithOrder> fwo;
    // 构建fwo
    if(!select_stmt->get_group_by_fields().empty()){
      for(const auto & f:select_stmt->get_group_by_fields()){
        fwo.push_back(FieldWithOrder(f, true));
      }
    }else{
      // 按第一个字段排序
      auto& [table_name, table_src] = *(table_map.begin());
      FieldIdentifier first_fid;
      if (auto table = std::get_if<Table*>(&table_src))
        first_fid = FieldIdentifier(table_name, (*table)->table_meta().field(0)->name());
      else {
        ResolvedView &resolved_view = std::get<ResolvedView>(table_src);
        first_fid = resolved_view.select_->select_expr_fields()[0].tuple_cell_spec_;
        fwo.push_back(FieldWithOrder(first_fid, true));
      }
    }

    unique_ptr<LogicalOperator> sort_oper = make_unique<SortLogicalOperator>(fwo);
    opers.push_back(std::move(sort_oper));
  }
  

  auto& agg_infos = select_stmt->fetch_all_agg_infos();
  for(auto&  agg_info: agg_infos){
    unique_ptr<LogicalOperator> agg_oper = make_unique<AggregateLogicalOperator>(agg_info.agg_field(), agg_info.full_name(), select_stmt->get_group_by_fields(), agg_info.agg_op());
    opers.push_back(std::move(agg_oper));
  }

  // deduplicate
  if(select_stmt->has_agg()){
    if(select_stmt->get_group_by_fields().empty()){
      // 无group by，只输出一条
      unique_ptr<LogicalOperator> deduplicate_oper = make_unique<DeduplicateLogicalOperator>(select_stmt->get_group_by_fields(), true);
      opers.push_back(std::move(deduplicate_oper));
    }else{
      unique_ptr<LogicalOperator> deduplicate_oper = make_unique<DeduplicateLogicalOperator>(select_stmt->get_group_by_fields(), false);
      opers.push_back(std::move(deduplicate_oper));
    }
  }

  for(auto &column_info : select_expr_fields){
    unique_ptr<LogicalOperator> fwc_oper = make_unique<FieldCulLogicalOperator>(column_info.tuple_cell_spec_, std::move(column_info.expr_));
    opers.push_back(std::move(fwc_oper));
    all_field_identifiers.push_back(column_info.tuple_cell_spec_);
    all_output_names.push_back(column_info.output_name_);
  }

  // having
  if (select_stmt->has_having_clause()) {
    unique_ptr<LogicalOperator> having_pred_oper(nullptr);
    rc = create_plan(select_stmt->fetch_having_exprs(), having_pred_oper);
    HANDLE_RC(rc);
    opers.push_back(std::move(having_pred_oper));
  }

  // order by
  unique_ptr<LogicalOperator> sort_oper = nullptr;
  if(!select_stmt->order_fields().empty()){
    sort_oper = make_unique<SortLogicalOperator>(select_stmt->order_fields());
  }
  opers.push_back(std::move(sort_oper));

  // proj: 当前的proj中包含group-by / field-cul-expr构建的tep_field
  unique_ptr<LogicalOperator> project_oper(new ProjectLogicalOperator(all_field_identifiers, all_output_names));
  opers.push_back(std::move(project_oper));

  // top_oper
  auto top_oper = create_select_oper_tree(opers);
  
  logical_operator.swap(top_oper);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    FilterStmt *filter_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  std::vector<unique_ptr<Expression>> cmp_exprs;
  const std::vector<FilterUnit *> &filter_units = filter_stmt->filter_units();
  for (const FilterUnit *filter_unit : filter_units) {
    const FilterObj &filter_obj_left = filter_unit->left();
    const FilterObj &filter_obj_right = filter_unit->right();

    unique_ptr<Expression> left(filter_obj_left.is_attr
                                         ? static_cast<Expression *>(new FieldExpr(filter_obj_left.field))
                                         : static_cast<Expression *>(new ValueExpr(filter_obj_left.value)));

    unique_ptr<Expression> right(filter_obj_right.is_attr
                                          ? static_cast<Expression *>(new FieldExpr(filter_obj_right.field))
                                          : static_cast<Expression *>(new ValueExpr(filter_obj_right.value)));

    ComparisonExpr *cmp_expr = new ComparisonExpr(filter_unit->comp(), std::move(left), std::move(right));
    cmp_exprs.emplace_back(cmp_expr);
  }

  unique_ptr<PredicateLogicalOperator> predicate_oper;
  // 这里为什么cmp_exprs会为空呢？哪种语句解析出来这边是空？
  if (!cmp_exprs.empty()) {
    unique_ptr<ConjunctionExpr> conjunction_expr(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));
    predicate_oper = unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(conjunction_expr)));
  }

  logical_operator = std::move(predicate_oper);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(std::unique_ptr<ConjunctionExpr> cond_exprs, std::unique_ptr<LogicalOperator> &logical_operator)
{
  logical_operator = std::make_unique<PredicateLogicalOperator>(std::move(cond_exprs));
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    InsertStmt *insert_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table *table = insert_stmt->table();
  vector<vector<Value>> value_rows(insert_stmt->value_rows());

  InsertLogicalOperator *insert_operator = new InsertLogicalOperator(table, value_rows);
  logical_operator.reset(insert_operator);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    DeleteStmt *delete_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table *table = delete_stmt->table();
  std::vector<Field> fields;
  // 把table中所有field都放到field中
  for (int i = table->table_meta().sys_field_num(); i < table->table_meta().field_num(); i++) {
    const FieldMeta *field_meta = table->table_meta().field(i);
    fields.push_back(Field(table, field_meta));
  }
  unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, table->name(), false/*readonly*/));

  unique_ptr<LogicalOperator> predicate_oper;
  RC rc = create_plan(delete_stmt->fetch_cond_exprs(), predicate_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  unique_ptr<LogicalOperator> delete_oper(new DeleteLogicalOperator(table));

  if (predicate_oper) {
    // delOp->predOp->tabOp
    predicate_oper->add_child(std::move(table_get_oper));
    delete_oper->add_child(std::move(predicate_oper));
    // delete
    //   |
    // predicate
    //   |
    // table_get
  } else {
    delete_oper->add_child(std::move(table_get_oper));
  }

  logical_operator = std::move(delete_oper);
  return rc;
}

RC LogicalPlanGenerator::create_plan(
    UpdateStmt *update_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table *table = update_stmt->table();
  std::vector<Field> fields;
  for (int i = table->table_meta().sys_field_num(); i < table->table_meta().field_num(); i++) {
    const FieldMeta *field_meta = table->table_meta().field(i);
    fields.push_back(Field(table, field_meta));
  }
  unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, table->name(), false/*readonly*/));

  unique_ptr<LogicalOperator> predicate_oper;
  RC rc = create_plan(update_stmt->fetch_cond_exprs(), predicate_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  std::vector<std::unique_ptr<LogicalOperator>> apply_in_set_stmts;
  std::vector<std::unique_ptr<ApplyStmt>> &subquerys_in_set_stmts = update_stmt->fetch_subquerys_in_set_stmts();
  for (auto &subquery: subquerys_in_set_stmts)
  {
    std::unique_ptr<LogicalOperator> apply_op;
    rc = subquery->create_logic_plan(apply_op);
    if (rc != RC::SUCCESS)
    {
      LOG_WARN("create logical plan for subquerys in set stmts failed, rc=%s.", strrc(rc));
      return rc;
    }
    apply_in_set_stmts.emplace_back(std::move(apply_op));
  }

  std::unordered_map<int, std::unique_ptr<Expression>> &value_list = update_stmt->fetch_value_list();
  unique_ptr<LogicalOperator> update_oper(new UpdateLogicalOperator(table, std::move(value_list), std::move(apply_in_set_stmts)));

  if (predicate_oper) {
    predicate_oper->add_child(std::move(table_get_oper));
    update_oper->add_child(std::move(predicate_oper));
  } else {
    update_oper->add_child(std::move(table_get_oper));
  }

  logical_operator = std::move(update_oper);
  return rc;
}

RC LogicalPlanGenerator::create_plan(
    ExplainStmt *explain_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Stmt *child_stmt = explain_stmt->child();
  unique_ptr<LogicalOperator> child_oper;
  RC rc = create(child_stmt, child_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create explain's child operator. rc=%s", strrc(rc));
    return rc;
  }

  logical_operator = unique_ptr<LogicalOperator>(new ExplainLogicalOperator);
  logical_operator->add_child(std::move(child_oper));
  return rc;
}

RC LogicalPlanGenerator::create_plan(CreateTableSelectStmt *cts_stmt, std::unique_ptr<LogicalOperator> &logical_operator){
  unique_ptr<LogicalOperator> select_top_oper(nullptr);
  RC rc = RC::SUCCESS;
  rc = create(cts_stmt->original_table_stmt(), select_top_oper);
  HANDLE_RC(rc);

  assert(select_top_oper.get());
  unique_ptr<InsertMultiLogicalOperator> insert_multi_oper = make_unique<InsertMultiLogicalOperator>();
  insert_multi_oper->add_child(std::move(select_top_oper));

  logical_operator = std::move(insert_multi_oper);
  return RC::SUCCESS;
}