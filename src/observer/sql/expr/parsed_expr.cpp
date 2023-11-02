#include "sql/expr/parsed_expr.h"
#include "sql/expr/expression.h"
#include "sql/stmt/apply_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "storage/table/table.h"
#include "storage/db/db.h"
#include "session/session.h"
#include "common/log/log.h"

RC StmtResolveContext::resolve_table_field_identifier(const std::string &table_name, const std::string &field_name,
    std::optional<FieldIdentifier> &result, ExprValueAttr *result_attr) const
{
    auto field_meta_to_value_attr = [](const FieldMeta *field_meta) {
        ExprValueAttr value_attr;
        value_attr.type = field_meta->type();
        value_attr.length = field_meta->len();
        value_attr.nullable = field_meta->nullable();
        return value_attr;
    };

    result = std::nullopt;
    ExprValueAttr value_attr;

    /* 给定了表名，则只查找该表 */
    if (!table_name.empty())
    {
        if (table_namespace_.count(table_name) != 0)
        {
            Table *table = table_namespace_.at(table_name);
            const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
            if (field_meta != nullptr)
            {
                result = FieldIdentifier(table_name, field_name);
                value_attr = field_meta_to_value_attr(field_meta);
            }
        }
        else if (views_.count(table_name) != 0)
        {
            auto &view_columns = views_.at(table_name);
            if (view_columns.count(field_name) != 0)
            {
                auto &[field_id, field_attr] = view_columns.at(field_name);
                result = field_id;
                value_attr = field_attr;
            }
        }
    }

    /* 只给定了列名，则遍历所有的表，查看是否有该列名，并且不允许冲突 */
    else
    {
        bool field_find = false;
        for (auto &[table_name, table]: table_namespace_)  // 在表中查找
        {
            const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
            if (field_meta != nullptr)
            {
                if (field_find)  // 多个表都有field_name这个字段，返回解析失败，而不是找不到
                {
                    LOG_WARN("field name %s is ambiguous", field_name.c_str());
                    return RC::SCHEMA_FIELD_AMBIGUOUS;
                }
                field_find = true;
                result = FieldIdentifier(table_name, field_name);
                value_attr = field_meta_to_value_attr(field_meta);
            }
        }
        
        for (auto &[view_name, view_columns]: views_)  // 在视图中查找
        {
            if (view_columns.count(field_name) != 0)
            {
                if (field_find)
                {
                    LOG_WARN("field name %s is ambiguous", field_name.c_str());
                    return RC::SCHEMA_FIELD_AMBIGUOUS;
                }
                field_find = true;
                auto &[field_id, field_attr] = view_columns.at(field_name);
                result = field_id;
                value_attr = field_attr;
            }
        }
    }

    if (result_attr != nullptr)
        *result_attr = value_attr;
    return RC::SUCCESS;
}

RC StmtResolveContext::resolve_field_referred_table(const std::string &field_name, std::string &result) const
{
    bool field_find = false;
    for (auto &[table_name, table]: table_namespace_)  // 在表中查找
    {
        const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
        if (field_meta != nullptr)
        {
            if (field_find)  // 多个表都有field_name这个字段，返回解析失败，而不是找不到
            {
                LOG_WARN("field name %s is ambiguous", field_name.c_str());
                return RC::SCHEMA_FIELD_AMBIGUOUS;
            }
            field_find = true;
            result = table_name;
        }
    }
    
    for (auto &[view_name, view_columns]: views_)  // 在视图中查找
    {
        if (view_columns.count(field_name) != 0)
        {
            if (field_find)
            {
                LOG_WARN("field name %s is ambiguous", field_name.c_str());
                return RC::SCHEMA_FIELD_AMBIGUOUS;
            }
            field_find = true;
            result = view_name;
        }
    }

    return RC::SUCCESS;
}

RC StmtResolveContext::resolve_other_column_name(const std::string &col_name) const
{
    if (other_column_names_.count(col_name))
        return RC::SUCCESS;
    else
        return RC::SCHEMA_FIELD_MISSING;
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

void ExprResolveResult::add_subquery(std::unique_ptr<ApplyStmt> subquery)
{
    subquerys_.emplace_back(std::move(subquery));
}

void ExprResolveResult::add_subquerys(std::vector<std::unique_ptr<ApplyStmt>> &subquerys)
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
        ExprValueAttr value_attr;
        RC rc = stmt_ctx.resolve_table_field_identifier(table_name, field_name, field_identifier, &value_attr);
        if (rc != RC::SUCCESS)
            return rc;
        
        // 找到了identifier对应的列名
        if (field_identifier.has_value())
        {
            // 如果是在当前作用域中搜索到的，也就是标识符引用的是当前层的表，那么不是相关表达式，就是一个IdentifierExpr
            if (level == ctx->current_level())
            {
                auto id_expr = std::make_unique<IdentifierExpr>(field_identifier.value());
                id_expr->set_value_attr(value_attr);
                if (!table_name.empty())
                    id_expr->set_referred_table(table_name);
                else
                {
                    std::string refer_table;
                    stmt_ctx.resolve_field_referred_table(field_name, refer_table);
                    id_expr->set_referred_table(refer_table);
                }
                expr.reset(id_expr.release());
            }
            // 否则，这个标识符是一个相关表达式，是第level层查询的相关子查询
            else
            {
                auto correlate_expr = std::make_unique<CorrelateExpr>(field_identifier.value());
                correlate_expr->set_value_attr(value_attr);
                if (!table_name.empty())
                    correlate_expr->set_referred_table(table_name);
                else
                {
                    std::string refer_table;
                    stmt_ctx.resolve_field_referred_table(field_name, refer_table);
                    correlate_expr->set_referred_table(refer_table);
                }
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
    result->add_agg_expr_infos(std::move(sub_result.get_agg_expr_infos()));

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
    result->add_agg_expr_infos(std::move(left_result.get_agg_expr_infos()));
    result->add_agg_expr_infos(std::move(right_result.get_agg_expr_infos()));

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
    result->add_agg_expr_infos(std::move(left_result.get_agg_expr_infos()));
    result->add_agg_expr_infos(std::move(right_result.get_agg_expr_infos()));

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
    result->add_agg_expr_infos(std::move(sub_result.get_agg_expr_infos()));

    return RC::SUCCESS;
}

RC ScalarSubqueryExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    SelectSqlNode &select_sql = sql_->selection;
    auto apply_stmt = std::make_unique<ScalarSubqueryApplyStmt>();
    Stmt *select_stmt = nullptr;
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_exprs_in_subquery;

    /* 递归解析子查询 */
    // 注：current_session会在communicator中使用default_session新建，传入SessionEvent，在session_stage保存到本线程的current_session中
    RC rc = SelectStmt::create(Session::current_session()->get_current_db(), ctx, select_sql, select_stmt, 
        &correlate_exprs_in_subquery);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("resolve subquery failed. rc=%d", strrc(rc));
        return rc;
    }

    /* 将子查询中，引用本层作用域的相关表达式放到ApplyStmt中 */
    if (correlate_exprs_in_subquery.count(ctx->current_level()))
    {
        apply_stmt->add_correlate_exprs(correlate_exprs_in_subquery[ctx->current_level()]);
        correlate_exprs_in_subquery.erase(ctx->current_level());
    }
    result->add_correlate_exprs(correlate_exprs_in_subquery); // 将子查询中，引用上层作用域的相关表达式放到ExprResolveResult中

    /* 将标量子查询转换为一个Identifier表达式 */
    FieldIdentifier id = ctx->get_subquery_identifier_generator().generate_identifier();
    result->set_result_expr_tree(std::make_unique<IdentifierExpr>(id));

    /* 构造apply_stmt的其它基础信息 */
    apply_stmt->set_sub_query_stmt(std::unique_ptr<SelectStmt>(dynamic_cast<SelectStmt*>(select_stmt)));
    apply_stmt->set_field_identifier_in_expr(id);
    result->add_subquery(std::move(apply_stmt));  // 将代表子查询的apply_stmt加入result结果

    return RC::SUCCESS;
}

RC ExistentialSubqueryExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    SelectSqlNode &select_sql = sql_->selection;
    auto apply_stmt = std::make_unique<ExistentialSubqueryApplyStmt>(op_);
    Stmt *select_stmt = nullptr;
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_exprs_in_subquery;

    /* 递归解析子查询 */
    RC rc = SelectStmt::create(Session::current_session()->get_current_db(), ctx, select_sql, select_stmt, 
        &correlate_exprs_in_subquery);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("resolve subquery failed. rc=%d", strrc(rc));
        return rc;
    }

    /* 将子查询中，引用本层作用域的相关表达式放到ApplyStmt中 */
    if (correlate_exprs_in_subquery.count(ctx->current_level()))
    {
        apply_stmt->add_correlate_exprs(correlate_exprs_in_subquery[ctx->current_level()]);
        correlate_exprs_in_subquery.erase(ctx->current_level());
    }
    result->add_correlate_exprs(correlate_exprs_in_subquery); // 将子查询中，引用上层作用域的相关表达式放到ExprResolveResult中

    /* 将存在性检测子查询转换为一个identifier = True的比较表达式 */
    FieldIdentifier id = ctx->get_subquery_identifier_generator().generate_identifier();
    auto identifier_expr = std::make_unique<IdentifierExpr>(id);
    auto value_expr = std::make_unique<ValueExpr>(Value(true));
    auto compare_expr = std::make_unique<ComparisonExpr>(CompOp::EQUAL_TO, std::move(identifier_expr), 
        std::move(value_expr));

    result->set_result_expr_tree(std::move(compare_expr));

    /* 构造apply_stmt的其它基础信息 */
    apply_stmt->set_sub_query_stmt(std::unique_ptr<SelectStmt>(dynamic_cast<SelectStmt*>(select_stmt)));
    apply_stmt->set_field_identifier_in_expr(id);
    result->add_subquery(std::move(apply_stmt));  // 将代表子查询的apply_stmt加入result结果

    return RC::SUCCESS;
}

RC QuantifiedCompSubqueryExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    /* Expr IN/NOT IN (sub_query)，解析Expr */
    ExprResolveResult child_result;
    RC rc = child_->resolve(ctx, &child_result);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("resolve child expr in quantified compare subquery failed. rc=%d", strrc(rc));
        return rc;
    }
    result->add_correlate_exprs(child_result.get_correlate_exprs());
    result->add_agg_expr_infos(std::move(child_result.get_agg_expr_infos()));

    /* Expr中的子查询要先于右边的(sub_query)执行，所以要先把它们添加进结果 */
    result->add_subquerys(child_result.get_subquerys_in_expr());

    /* Expr IN/NOT IN (sub_query)，解析右边的子查询 */
    SelectSqlNode &select_sql = sql_->selection;
    auto apply_stmt = std::make_unique<QuantifiedCompSubqueryApplyStmt>(child_result.owns_result_expr_tree(), op_);
    Stmt *select_stmt = nullptr;
    std::unordered_map<size_t, std::vector<CorrelateExpr*>> correlate_exprs_in_subquery;

    rc = SelectStmt::create(Session::current_session()->get_current_db(), ctx, select_sql, select_stmt, 
        &correlate_exprs_in_subquery);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("resolve subquery failed. rc=%d", strrc(rc));
        return rc;
    }

    /* 将子查询中，引用本层作用域的相关表达式放到ApplyStmt中 */
    if (correlate_exprs_in_subquery.count(ctx->current_level()))
    {
        apply_stmt->add_correlate_exprs(correlate_exprs_in_subquery[ctx->current_level()]);
        correlate_exprs_in_subquery.erase(ctx->current_level());
    }
    result->add_correlate_exprs(correlate_exprs_in_subquery); // 将子查询中，引用上层作用域的相关表达式放到ExprResolveResult中

    /* 将集合比较子查询转换为一个identifier = True的比较表达式 */
    FieldIdentifier id = ctx->get_subquery_identifier_generator().generate_identifier();
    auto identifier_expr = std::make_unique<IdentifierExpr>(id);
    auto value_expr = std::make_unique<ValueExpr>(Value(true));
    auto compare_expr = std::make_unique<ComparisonExpr>(CompOp::EQUAL_TO, std::move(identifier_expr), 
        std::move(value_expr));

    result->set_result_expr_tree(std::move(compare_expr));

    /* 构造apply_stmt的其它基础信息 */
    apply_stmt->set_sub_query_stmt(std::unique_ptr<SelectStmt>(dynamic_cast<SelectStmt*>(select_stmt)));
    apply_stmt->set_field_identifier_in_expr(id);
    result->add_subquery(std::move(apply_stmt));  // 代表集合比较子查询的Apply最后加入结果

    return RC::SUCCESS;
}

RC AggIdentifierExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    RC rc = RC::SUCCESS;
    bool is_in_select_column = false;
    std::optional<FieldIdentifier> resolved_identifier;
    ExprValueAttr agg_field_attr;

    const StmtResolveContext& stmt_ctx = ctx->get_ith_level_stmt_ctx(ctx->current_level());
    
    /* 解析聚集函数中的agg_field是否合法，当前只在所属作用域内解析，不考虑引用外层查询的情况 */
    // 如果是count(*)，就不走stmt_ctx的解析，直接是合法的
    if (agg_field_.field_name() == "*")
    {
        if (op_ != AggregateOp::AGG_COUNT)
        {
            LOG_WARN("only count(*) is supported.");
            return RC::SCHEMA_FIELD_MISSING;
        }
        else
            resolved_identifier = agg_field_;
    }
    else
    {
        rc = stmt_ctx.resolve_table_field_identifier(agg_field_.table_name(), agg_field_.field_name(), resolved_identifier,
            &agg_field_attr);
        if (rc != RC::SUCCESS || !resolved_identifier.has_value())
        {
            LOG_WARN("resolve agg field failed.");
            return RC::SCHEMA_FIELD_MISSING;
        }
    }

    /* 如果聚集函数名在列中，做个标记，外部就不需要再生成一个agg算子了 */
    rc = stmt_ctx.resolve_other_column_name(expr_name_);
    if (rc == RC::SUCCESS)
        is_in_select_column = true;
    
    /* 把这个聚集函数表达式加入结果中 */
    result->add_agg_expr_info(AggExprInfo(op_, resolved_identifier.value(), expr_name_, is_in_select_column));

    /* 生成一个以聚集函数全名为Identifer的表达式，用于从tuple中取出聚集函数结果 */
    auto expr = std::make_unique<IdentifierExpr>(FieldIdentifier(expr_name_));

    ExprValueAttr value_attr;
    if (op_ == AggregateOp::AGG_COUNT)
    {
        value_attr.length = 4;
        value_attr.type = AttrType::INTS;
        value_attr.nullable = true;
    }
    else if (op_ == AggregateOp::AGG_AVG)
    {
        value_attr.length = 4;
        value_attr.type = AttrType::FLOATS;
        value_attr.nullable = true;
    }
    else  // MAX, MIN, SUM
    {
        value_attr = agg_field_attr;
    }
    expr->set_value_attr(value_attr);

    result->set_result_expr_tree(std::move(expr));
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
    result->add_agg_expr_infos(std::move(sub_result.get_agg_expr_infos()));

    return RC::SUCCESS;
}

RC QuantifiedCmpExprSetExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const
{
    ExprResolveResult sub_result;
    RC rc = child_->resolve(ctx, &sub_result);
    if (rc != RC::SUCCESS)
        return rc;
    result->add_correlate_exprs(sub_result.get_correlate_exprs());
    result->add_subquerys(sub_result.get_subquerys_in_expr());
    result->add_agg_expr_infos(std::move(sub_result.get_agg_expr_infos()));

    std::vector<std::unique_ptr<Expression>> expr_for_set;
    for (auto &expr_in_set: *expr_set_)
    {
        ExprResolveResult res;
        rc = expr_in_set->resolve(ctx, &res);
        if (rc != RC::SUCCESS)
            return rc;
        expr_for_set.emplace_back(res.owns_result_expr_tree());
        result->add_correlate_exprs(sub_result.get_correlate_exprs());
        result->add_subquerys(sub_result.get_subquerys_in_expr());
    }

    auto expr_tree = std::make_unique<QuantifiedCompExprSetExpr>(sub_result.owns_result_expr_tree(), op_, 
        std::move(expr_for_set));
    result->set_result_expr_tree(std::move(expr_tree));
    return RC::SUCCESS;
}

RC FunctionExprSqlNode::resolve(ExprResolveContext *ctx, ExprResolveResult *result) const 
{
    std::unique_ptr<FunctionExpr> expr;
    ExprValueAttr value_attr;
    if (!func_sql_.is_const)
    {
        // 这里不考虑Function中的参数是相关表达式
        const StmtResolveContext& cur_ctx = ctx->get_ith_level_stmt_ctx(ctx->current_level());
        std::optional<FieldIdentifier> arg;
        RC rc = cur_ctx.resolve_table_field_identifier(func_sql_.rel_attr.relation_name, func_sql_.rel_attr.attribute_name, arg);
        if (rc != RC::SUCCESS || !arg.has_value())
        {
            LOG_WARN("resolve arg in function failed");
            return rc;
        }

        expr = std::make_unique<FunctionExpr>(func_sql_.function_kernel->copy(), arg.value());
        if (!func_sql_.rel_attr.relation_name.empty())
            expr->set_referred_table(func_sql_.rel_attr.relation_name);
        else
        {
            std::string refer_table;
            cur_ctx.resolve_field_referred_table(func_sql_.rel_attr.attribute_name, refer_table);
            expr->set_referred_table(refer_table);
        }
    }
    else
    {
        expr = std::make_unique<FunctionExpr>(func_sql_.function_kernel->copy());
    }
    
    result->set_result_expr_tree(std::move(expr));
    return RC::SUCCESS;
}