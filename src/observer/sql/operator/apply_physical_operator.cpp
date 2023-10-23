#include "sql/operator/apply_physical_operator.h"

ApplyPhysicalOperator::ApplyPhysicalOperator(const FieldIdentifier &identifier, 
    std::vector<CorrelateExpr*> &&correlate_exprs_in_subquery, std::unique_ptr<SubqueryExprEvaluator> subquery_expr_evaluator, 
    std::unique_ptr<PhysicalOperator> child_op, std::unique_ptr<PhysicalOperator> sub_query)
    : field_id_in_expr_(identifier), correlate_exprs_in_subquery_(std::move(correlate_exprs_in_subquery)), 
    subquery_expr_evaluator_(std::move(subquery_expr_evaluator))
{
    FieldMeta new_field_meta(identifier.field_name().c_str(), AttrType::INTS, 1, 1, true);
    tuple_.set_new_field_meta(new_field_meta);

    children_.emplace_back(std::move(child_op));
    children_.emplace_back(std::move(sub_query));
}

RC ApplyPhysicalOperator::open(Trx *trx)
{
    if (children_.size() != 2)
    {
        LOG_WARN("apply operator must have two children");
        return RC::INTERNAL;
    }
    trx_ = trx;

    RC rc = children_[child_op_idx]->open(trx);
    if (rc != RC::SUCCESS)
        return rc;

    /* 如果是无关子查询，那么在Apply算子open时直接物化 */
    if (!is_correlated_subquery())
        return execute_subquery();
    
    return RC::SUCCESS;
}

RC ApplyPhysicalOperator::close()
{
    return children_[child_op_idx]->close();
}

RC ApplyPhysicalOperator::next()
{
    RC rc = children_[child_op_idx]->next();
    if (rc != RC::SUCCESS)
        return rc;
    
    Tuple *child_tuple = children_[child_op_idx]->current_tuple();
    if (child_tuple == nullptr)
    {
        LOG_WARN("child operator returned null tuple while still has next.");
        return RC::INTERNAL;
    }

    /* 拿到外层tuple后，如果是相关子查询，就执行一遍子查询，如果非相关子查询，那么Apply算子在Open时已经做过了 */
    if (is_correlated_subquery())
    {
        rc = execute_subquery(*child_tuple);
        if (rc != RC::SUCCESS)
        {
            LOG_WARN("failed to execute subquery, rc=%s.", strrc(rc));
            return rc;
        }
    }

    /* 将子查询结果传入子查询表达式求值器，拿到对应的值 */
    Value value;
    rc = subquery_expr_evaluator_->evaluate(child_tuple, subquery_result_, value);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to evaluate subquery expr, rc=%s.", strrc(rc));
        return rc;
    }

    /* 使用外层tuple和子查询表达式的值构造新的tuple */
    tuple_.set_original(child_tuple);
    tuple_.set_new_field_value(value);

    return RC::SUCCESS;
}

Tuple *ApplyPhysicalOperator::current_tuple()
{
    return &tuple_;
}

RC ApplyPhysicalOperator::execute_subquery(const Tuple &current_outer_tuple)
{
    RC rc = RC::SUCCESS;

    /* 根据当前外层查询的tuple，计算出子查询中所有关联到此外层查询的相关表达式的值 */
    for (CorrelateExpr *expr : correlate_exprs_in_subquery_)
    {
        rc = expr->set_value_from_tuple(current_outer_tuple);
        if (rc != RC::SUCCESS)
        {
            LOG_WARN("failed to set value for correlate expr(identifier=%s), rc=%s", expr->identifier_name().c_str(), 
                strrc(rc));
            return rc;
        }
    }
    
    /* 现在子查询中的相关表达式的值都被设置好，可以执行了 */
    return execute_subquery_inner();
}

RC ApplyPhysicalOperator::execute_subquery_inner()
{
    subquery_result_.clear();
    RC rc = children_[sub_query_idx]->open(trx_);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("open sub query physical operator failed, rc=%s", strrc(rc));
        return rc;
    }

    while (true)
    {
        rc = children_[sub_query_idx]->next();
        if (rc == RC::SUCCESS)
        {
            Tuple *sub_query_tuple = children_[sub_query_idx]->current_tuple();
            subquery_result_.emplace_back(sub_query_tuple->get_replica());
        }
        else if (rc == RC::RECORD_EOF)
        {
            break;
        }
        else
        {
            LOG_WARN("next sub query physical operator failed, rc=%s", strrc(rc));
            return rc;
        }
    }

    return children_[sub_query_idx]->close();
}

RC ScalarSubqueryEvaluator::evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value)
{
    /* 如果子查询结果为空，则返回NULL */
    if (subquery_tuples.empty())
    {
        value.set_type(AttrType::NULL_TYPE);
        return RC::SUCCESS;
    }

    /* 如果子查询不是标量子查询，那么返回错误 */
    if (subquery_tuples.size() > 1 || subquery_tuples[0]->cell_num() != 1)
    {
        LOG_WARN("subquery is not a scalar subquery");
        return RC::SUBQUERY_NOT_SCALAR;
    }

    RC rc = subquery_tuples[0]->cell_at(0, value);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to get value from subquery");
        return RC::SUBQUERY_GET_VALUE_FAILED;
    }

    return RC::SUCCESS;
}

RC ExistentialSubqueryEvaluator::evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value)
{
    bool is_exist = !subquery_tuples.empty();
    value.set_boolean(op_ == EXISTS ? is_exist : !is_exist);
    return RC::SUCCESS;
}

RC QuantifiedCompSubqueryEvaluator::evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value)
{
    Value expr_value;
    RC rc = expr_->get_value(*current_outer_tuple, expr_value);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to get expr value in quatified compare subquery, rc=%s", strrc(rc));
        return rc;
    }

    bool is_in = false;
    for (auto &tuple: subquery_tuples)
    {
        /* 如果subquery返回的tuple有多列，则错误 */
        if (tuple->cell_num() > 1)
        {
            LOG_WARN("subquery returns multi columns in quantified compare subquery.");
            return RC::SUBQUERY_NOT_SCALAR;
        }

        Value tuple_value;
        rc = tuple->cell_at(0, tuple_value);
        if (rc != RC::SUCCESS)
        {
            LOG_WARN("failed to get value from subquery");
            return RC::SUBQUERY_GET_VALUE_FAILED;
        }

        if (expr_value.compare(tuple_value) == 0)
        {
            is_in = true;
            break;
        }
    }

    value.set_boolean(op_ == IN ? is_in : !is_in);
    return RC::SUCCESS;
}