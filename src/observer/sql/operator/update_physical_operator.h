#pragma once

#include <vector>
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse.h"
#include "sql/expr/tuple.h"

class UpdateStmt;

/* 
 * Update物理算子中，向set语句中的表达式提供tuple的适配算子
 * Update物理算子从下层拿到RowTuple后，同时将这个Tuple提供给此算子(如果存在)，
 * 此算子或此算子所在的Apply树计算出现在set中的子查询的值，
 * 
 * 为什么这么做：
 * 计算表达式值的算子树(T)无法直接拿到RowTuple，因为这样会需要两个TableGet，分别给Update算子树和T，它们还不一定同步
 * 因此，T不是出现在Update算子树中，而是在UpdatePhysicalOperator内部，
 * UpdatePhysicalOperator拿到下层的RowTuple后，再通过这个适配算子复制一份RowTuple提供给T
 */
class AdaptorPhysicalOperatorForExprInSetStmt: public PhysicalOperator
{
public:
  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::ADAPTOR_FOR_EXPR_IN_UPDATE_SET;
  }

  RC open(Trx *trx) override { 
    tuple_.reset(nullptr);
    return RC::SUCCESS; 
  }
  RC next() override { return RC::SUCCESS; }
  RC close() override { return RC::SUCCESS; }
  
  Tuple *current_tuple() override { return tuple_.get(); }

  // 向Adaptor提供新算子
  void owns_new_tuple(Tuple *tuple) { tuple_.reset(tuple); }

private:
  std::unique_ptr<Tuple> tuple_;
};

/**
 * @brief 插入物理算子 更新
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table,std::unordered_map<int, std::unique_ptr<Expression>> &&value_list,
    std::unique_ptr<PhysicalOperator> tuple_calculator_for_expr_in_set,
    AdaptorPhysicalOperatorForExprInSetStmt *adaptor_for_tuple_calculator)
    : table_(table), value_list_(std::move(value_list)), 
      tuple_calculator_for_expr_in_set_(std::move(tuple_calculator_for_expr_in_set)),
      adaptor_for_tuple_calculator_(adaptor_for_tuple_calculator) {};

  virtual ~UpdatePhysicalOperator() = default;

  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::UPDATE;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override { return nullptr; }

private:
  Table *table_ = nullptr;
  std::unordered_map<int, std::unique_ptr<Expression>> value_list_;
  Trx *trx_ = nullptr;

  // 如果set语句中有子查询，则存在以下数据结构：
  std::unique_ptr<PhysicalOperator> tuple_calculator_for_expr_in_set_;

  // 该算子处于tuple_calculator_for_expr_in_set(T)的最底部，或者就是T本身，所有权归T管
  // 这里需要一个引用，因为UpdatePhysicalOperator要把从下层获得的RowTuple复制一份传到这个算子中
  // 然后这个算子使用该Tuple计算set中各表达式所需要的值
  AdaptorPhysicalOperatorForExprInSetStmt *adaptor_for_tuple_calculator_;
};
