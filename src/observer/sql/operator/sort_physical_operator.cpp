#include "sql/operator/sort_physical_operator.h"
#include "sql/expr/tuple_cell.h"
#include <algorithm>
#include <execution>
#include <pstl/glue_execution_defs.h>

RC SortPhysicalOperator::open(Trx *trx)
{
  RC                rc   = RC::SUCCESS;
  PhysicalOperator *oper = children_.front().get();
  rc = oper->open(trx);
  if(rc != RC::SUCCESS){
    return rc;
  }
  while (RC::SUCCESS == (rc = oper->next())) {
    Tuple *tuple = oper->current_tuple();
    tuples_.push_back(tuple->get_replica());
  }
  generate_cmp_functions();
  // 为啥(const Tuple *&l_tuple, const Tuple *&r_tuple)加了引用就不行
  auto cmp_func = std::bind(&SortPhysicalOperator::cmp_function, this, std::placeholders::_1, std::placeholders::_2);
  std::sort(std::execution::par_unseq, tuples_.begin(), tuples_.end(), cmp_func);
  it = --tuples_.begin();  // 未定义行为！
  return RC::SUCCESS;
}

// 因为算子是先next再current_tuple，所以和迭代器的逻辑错开了一位，不然第一个元素不会被遍历
RC SortPhysicalOperator::next()
{
  it++;
  if (it == tuples_.end()) {
    return RC::RECORD_EOF;
  }
  return RC::SUCCESS;
}

RC SortPhysicalOperator::close() { 
  if(!tuples_.empty()){
    for(auto& tuple : tuples_){
      delete tuple;
    }
  }
  tuples_.clear();
  return children_[0]->close(); 
}

Tuple *SortPhysicalOperator::current_tuple()
{
  return *it;
}

bool SortPhysicalOperator::cmp_function(const Tuple *l_tuple, const Tuple *r_tuple)
{
  for (auto &func : cmp_functions_) {
    bool equal;
    bool cmp_result = func(l_tuple, r_tuple, equal);
    if (equal) {
      // 值相同，看下一个字段比较
      continue;
    } else {
      // 这里面返回值已经按照order规范好了
      return cmp_result;
    }
  }
  // 当前没做完整性约束，可能会出现每个字段都相同的记录
  return false;
}

class SortPhysicalOperator::CmpFunctor
{
public:
  CmpFunctor(FieldWithOrder fwo) : fwo_(fwo) {}
  // 最后传出一个bool值的原因是：cpp中sort的重载比较函数只能返回bool值，但两个value相等的时候我们要比较下个字段
  // 所以我们需要第三个状态"equal"。其实把返回值改成int在上层判断也行，但是我想这一个仿函数解决所有问题
  bool operator()(const Tuple *l_tuple, const Tuple *r_tuple, bool &equal)  //从小到大
  {
    equal = false;
    // 1. 获得value: 如果table_name没被初始化怎么办，要不要在resolver中初始化他
    Value l_v, r_v;
    l_tuple->find_cell(TupleCellSpec(fwo_.field.table_name(), fwo_.field.field_name()), l_v);
    r_tuple->find_cell(TupleCellSpec(fwo_.field.table_name(), fwo_.field.field_name()), r_v);
    // 2. Null的特判
    int cmp_result = l_v.compare(r_v);
    if (cmp_result == 0) {
      equal = true;
      return false;
    }
    if (fwo_.get_is_acs()) {
      return (l_v.compare(r_v) > 0) ? false : true;
    } else {
      return (l_v.compare(r_v) > 0) ? true : false;
    }
  }

private:
  FieldWithOrder fwo_;
};

bool SortPhysicalOperator::generate_cmp_functions()
{
  for (auto fwo : order_fields_) {
    cmp_functions_.push_back(CmpFunctor(fwo));
  }
  return true;
}

SortPhysicalOperator::~SortPhysicalOperator(){
  if(!tuples_.empty()){
    for(auto& tuple : tuples_){
      delete tuple;
    }
  }
  tuples_.clear();
}