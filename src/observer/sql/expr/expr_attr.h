#pragma once

#include "sql/parser/parse_defs.h"

struct ExprValueAttr
{
  AttrType type = AttrType::INTS;
  size_t length = 4;
  bool nullable = true;
};