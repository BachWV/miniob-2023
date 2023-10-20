#pragma once

#include <string>
#include <vector>
#include <memory>

class CorrelateExpr;
class SelectStmt;

/* 
 * 包含构造Apply算子的全部信息
 * Apply算子，对应一个返回标量值的，包含子查询的表达式
 * 在resolver阶段，包含子查询的表达式用一个field代替，表达式转化为ApplyStmt，用于在logic_plan阶段构造apply算子 
 */
class ApplyStmt
{
public:
    ApplyStmt() = default;
    virtual ~ApplyStmt() = default;

    void set_field_name_in_expr(const std::string &name) { field_name_in_expr_ = name; }
    void set_sub_query_stmt(std::unique_ptr<SelectStmt> sub_query);
    void add_correlate_expr(CorrelateExpr *expr);

protected:
    std::string field_name_in_expr_;  // 替代此表达式的field名字
    std::unique_ptr<SelectStmt> sub_query_;  // 子查询的SelectStmt
    std::vector<CorrelateExpr*> correlate_exprs_;  // 子查询中引用ApplyStmt所属查询的表达式
};