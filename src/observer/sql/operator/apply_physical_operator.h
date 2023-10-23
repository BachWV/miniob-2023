#include "sql/operator/apply_logical_operator.h"
#include "sql/operator/physical_operator.h"

class SubqueryExprEvaluator
{
public:
    virtual RC evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value) = 0;
};

/* 标量子查询表达式，返回的值取决于子查询的值 */
class ScalarSubqueryEvaluator: public SubqueryExprEvaluator
{
public:
    RC evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value) override;
};

/* 存在性检测子查询表达式 */
class ExistentialSubqueryEvaluator: public SubqueryExprEvaluator
{
public:
    ExistentialSubqueryEvaluator(ExistOp op): op_(op) {}

    RC evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value) override;

protected:
    ExistOp op_;
};

class QuantifiedCompSubqueryEvaluator: public SubqueryExprEvaluator
{
public:
    QuantifiedCompSubqueryEvaluator(std::unique_ptr<Expression> expr, QuantifiedComp op)
        : expr_(std::move(expr)), op_(op) {}

    RC evaluate(Tuple *current_outer_tuple, const std::vector<std::unique_ptr<Tuple>> &subquery_tuples, 
        Value &value) override;

protected:
    QuantifiedComp op_;
    std::unique_ptr<Expression> expr_;  // expr in subquery的expr
};

class ApplyPhysicalOperator : public PhysicalOperator
{
public:
    ApplyPhysicalOperator(const FieldIdentifier &identifier, std::vector<CorrelateExpr*> &&correlate_exprs_in_subquery, 
        std::unique_ptr<SubqueryExprEvaluator> subquery_expr_evaluator,
        std::unique_ptr<PhysicalOperator> child_op, std::unique_ptr<PhysicalOperator> sub_query);

    PhysicalOperatorType type() const override { return PhysicalOperatorType::APPLY; }

    RC open(Trx *trx) override;
    RC next() override;
    RC close() override;

    Tuple *current_tuple() override;

protected:
    bool is_correlated_subquery() const { return correlate_exprs_in_subquery_.size() != 0; }
    RC execute_subquery() { return execute_subquery_inner(); }  // 执行无关子查询
    RC execute_subquery(const Tuple &current_outer_tuple);  // 执行相关子查询
    RC execute_subquery_inner();

protected:
    FieldIdentifier field_id_in_expr_;  // Apply算子对应子查询表达式被替换为identifier
    std::vector<CorrelateExpr*> correlate_exprs_in_subquery_;  // 子查询中，引用Apply算子所在层的相关表达式
    std::unique_ptr<SubqueryExprEvaluator> subquery_expr_evaluator_;  // 子查询表达式求值器
    AddOneFieldTuple tuple_;  // 下层算子返回的Tuple，加上一个子查询表达式值字段，字段名为field_id_in_expr_

    std::vector<std::unique_ptr<Tuple>> subquery_result_;  // 物化子查询的结果

    Trx *trx_;

    constexpr static size_t child_op_idx = 0, sub_query_idx = 1;
};