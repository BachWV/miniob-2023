#include "sql/expr/parsed_expr.h"
#include "sql/expr/expression.h"
#include "sql/stmt/apply_stmt.h"
#include "storage/table/table.h"

RC StmtResolveContext::resolve_table_field_identifier(const std::string &table_name, const std::string &field_name,
    std::optional<FieldIdentifier> &result) const
{
    result = std::nullopt;

    /* 给定了表名，则只查找该表 */
    if (!table_name.empty())
    {
        if (table_namespace_.count(table_name) == 0)
            return RC::SUCCESS;

        Table *table = table_namespace_.at(table_name);
        const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());

        if (field_meta != nullptr)
            result = FieldIdentifier(table_name, field_name);
        
        return RC::SUCCESS;
    }

    /* 只给定了列名，则遍历所有的表，查看是否有该列名，并且不允许冲突 */
    else
    {
        std::string target_table_name;
        const FieldMeta *target_field = nullptr;
        for (const auto &[ele_table_name, table]: table_namespace_) 
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

        if (target_field != nullptr)
            result = FieldIdentifier(target_table_name, field_name);
        return RC::SUCCESS;
    }
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<ApplyStmt> SubQueryInfo::owns_apply_stmt()
{
    return std::move(apply_stmt_);
}

///////////////////////////////////////////////////////////////////////////////

void ExprResolveResult::set_result_expr_tree(std::unique_ptr<Expression> result_expr_tree)
{
    result_expr_tree_ = std::move(result_expr_tree);
}

void ExprResolveResult::add_correlate_expr(size_t level, CorrelateExpr *expr)
{
    correlate_exprs_[level].push_back(expr);
}

void ExprResolveResult::add_correlate_exprs(std::unordered_map<size_t, std::vector<CorrelateExpr*>> &exprs)
{
    for (auto &[level, expr_vec]: exprs)
        correlate_exprs_[level].insert(correlate_exprs_[level].end(), expr_vec.begin(), expr_vec.end());
}

void ExprResolveResult::add_subquerys(std::vector<SubQueryInfo> &subquerys)
{
    for (auto &subquery: subquerys)
        subquerys_.emplace_back(std::move(subquery));
}

std::unordered_map<size_t, std::vector<CorrelateExpr*>> &ExprResolveResult::get_correlate_exprs()
{
    return correlate_exprs_;
}

std::unique_ptr<Expression> ExprResolveResult::owns_result_expr_tree()
{
    return std::move(result_expr_tree_);
}

///////////////////////////////////////////////////////////////////////////////

RC ValueExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    result->set_result_expr_tree(std::make_unique<ValueExpr>(value_));
    return RC::SUCCESS;
}

RC IdentifierExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    ssize_t level = ctx->current_level();
    bool resolve_success = false;
    std::unique_ptr<Expression> expr;

    // 从当前Select层级开始，不断向上解析名字
    for (; level >= 0; --level)
    {
        const StmtResolveContext& stmt_ctx = ctx->get_ith_level_stmt_ctx(level);

        /* Identifier类型的表达式只能是引用表中列名的表达式，其它的名字引用用其它类型的表达式实现 */
        std::optional<FieldIdentifier> field_identifier;
        RC rc = stmt_ctx.resolve_table_field_identifier(table_name, field_name, field_identifier);
        if (rc != RC::SUCCESS)
            return rc;
        
        // 找到了identifier对应的列名
        if (field_identifier.has_value())
        {
            // 如果是在当前作用域中搜索到的，也就是标识符引用的是当前层的表，那么不是相关表达式，就是一个IdentifierExpr
            if (level == ctx->current_level())
            {
                expr = std::make_unique<IdentifierExpr>(field_identifier.value());
            }
            // 否则，这个标识符是一个相关表达式，是第level层查询的相关子查询
            else
            {
                auto correlate_expr = std::make_unique<CorrelateExpr>(field_identifier.value());
                result->add_correlate_expr(level, correlate_expr.get());
                expr.reset(correlate_expr.release());
                LOG_DEBUG("add correlate expr : %s.%s . Correlate to level=%u, table=%s, field=%s", 
                    table_name.c_str(), field_name.c_str(), level, field_identifier->table_name().c_str(), 
                    field_identifier->field_name().c_str());
            }

            resolve_success = true;
            break;
        }
    }

    if (!resolve_success)
    {
        LOG_WARN("can not resolve identifier expr. table=%s, field=%s", table_name.c_str(), field_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
    }

    /* IdentifierExprSqlNode不是子查询，只用设置resolve之后的表达式即可，如果是一个相关表达式，刚刚已经使用add_correlate_expr添加进去了 */
    result->set_result_expr_tree(std::move(expr));
    return RC::SUCCESS;
}

RC NegativeArithExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    ExprResolveResult sub_result;
    RC rc = sub_expr_->resolve(ctx, &sub_result);
    if (rc != RC::SUCCESS)
        return rc;
    
    auto expr_tree = std::make_unique<ArithmeticExpr>(ArithmeticExpr::Type::NEGATIVE, sub_result.owns_result_expr_tree(), nullptr);
    result->set_result_expr_tree(std::move(expr_tree));
    result->add_correlate_exprs(sub_result.get_correlate_exprs());
    result->add_subquerys(sub_result.get_subquerys_in_expr());

    return RC::SUCCESS;
}

RC BinaryArithExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    static std::unordered_map<BinaryArithType, ArithmeticExpr::Type> op_map = {
        {BinaryArithType::Add, ArithmeticExpr::Type::ADD},
        {BinaryArithType::Sub, ArithmeticExpr::Type::SUB},
        {BinaryArithType::Mul, ArithmeticExpr::Type::MUL},
        {BinaryArithType::Div, ArithmeticExpr::Type::DIV}
    };

    ExprResolveResult left_result, right_result;
    RC rc = left_->resolve(ctx, &left_result);
    if (rc != RC::SUCCESS)
        return rc;
    rc = right_->resolve(ctx, &right_result);
    if (rc != RC::SUCCESS)
        return rc;
    
    auto expr_tree = std::make_unique<ArithmeticExpr>(op_map[type_], left_result.owns_result_expr_tree(), 
        right_result.owns_result_expr_tree());
    result->set_result_expr_tree(std::move(expr_tree));
    result->add_correlate_exprs(left_result.get_correlate_exprs());
    result->add_correlate_exprs(right_result.get_correlate_exprs());
    result->add_subquerys(left_result.get_subquerys_in_expr());
    result->add_subquerys(right_result.get_subquerys_in_expr());

    return RC::SUCCESS;
}

RC CompareExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    ExprResolveResult left_result, right_result;
    RC rc = left_->resolve(ctx, &left_result);
    if (rc != RC::SUCCESS)
        return rc;
    rc = right_->resolve(ctx, &right_result);
    if (rc != RC::SUCCESS)
        return rc;
    
    auto expr_tree = std::make_unique<ComparisonExpr>(op_, left_result.owns_result_expr_tree(), 
        right_result.owns_result_expr_tree());
    result->set_result_expr_tree(std::move(expr_tree));
    result->add_correlate_exprs(left_result.get_correlate_exprs());
    result->add_correlate_exprs(right_result.get_correlate_exprs());
    result->add_subquerys(left_result.get_subquerys_in_expr());
    result->add_subquerys(right_result.get_subquerys_in_expr());

    return RC::SUCCESS;
}

RC PredNullExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    ExprResolveResult sub_result;
    RC rc = sub_expr_->resolve(ctx, &sub_result);
    if (rc != RC::SUCCESS)
        return rc;
    
    auto expr_tree = std::make_unique<PredNullExpr>(op_, sub_result.owns_result_expr_tree());
    result->set_result_expr_tree(std::move(expr_tree));
    result->add_correlate_exprs(sub_result.get_correlate_exprs());
    result->add_subquerys(sub_result.get_subquerys_in_expr());
    return RC::SUCCESS;
}

RC SubQueryExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    /* TODO */
    return RC::SUCCESS;
}

RC LikeExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    ExprResolveResult sub_result;
    RC rc = left_->resolve(ctx, &sub_result);
    if (rc != RC::SUCCESS)
        return rc;
    auto expr_tree = std::make_unique<LikeExpr>(sub_result.owns_result_expr_tree(), pattern_, is_not_like_);
    result->set_result_expr_tree(std::move(expr_tree));
    result->add_correlate_exprs(sub_result.get_correlate_exprs());
    result->add_subquerys(sub_result.get_subquerys_in_expr());
    return RC::SUCCESS;
}