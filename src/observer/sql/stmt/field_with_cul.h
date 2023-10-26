#pragma once

#include "sql/parser/parse_defs.h"
#include "storage/field/field.h"
#include "storage/field/field_meta.h"
#include <memory>
struct FieldWithCul
{
public:
  FieldWithCul(std::string virtual_field_name, std::unique_ptr<Expression> &&cul_expr)
      : virtual_field_name_(virtual_field_name), cul_expr_(std::move(cul_expr))
  {}
  std::unique_ptr<Expression> cul_expr_;
  std::string                 virtual_field_name_;
  std::string                 alias_;
  Field                       get_tmp_field()
  {
    FieldMeta *meta = new FieldMeta(virtual_field_name_.c_str(), INTS, 1, 1, true, false);
    return Field(nullptr, meta);
  }
};