#pragma once

#include "storage/field/field.h"
#include <string>

struct FieldWithAlias{
  FieldWithAlias(Field f, std::string a): field_(f), alias_(a){}
  Field field_;
  std::string alias_;
};