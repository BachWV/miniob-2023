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
// Created by Wangyunlai on 2022/12/15
//

#include "sql/operator/table_get_logical_operator.h"

TableGetLogicalOperator::TableGetLogicalOperator(Table *table, const std::string &table_name, bool readonly)
    : table_(table), table_name_(table_name), readonly_(readonly)
{}

void TableGetLogicalOperator::set_predicates(std::vector<std::unique_ptr<Expression>> &&exprs)
{
  predicates_ = std::move(exprs);
}
