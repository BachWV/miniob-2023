#pragma once

#include "sql/parser/parse_defs.h"

struct ExprValueAttr
{
  AttrType type;
  size_t length;
  bool nullable;
};