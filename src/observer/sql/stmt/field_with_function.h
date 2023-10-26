#pragma once

#include "sql/function_kernel/function_kernel.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field.h"
#include <memory>

struct FieldWithFunction
{
  FieldWithFunction(std::string virtual_field_name, Field field, std::unique_ptr<FunctionKernel> &&fk)
      : virtual_field_name_(virtual_field_name), be_functioned_field_(field)
  {
    function_kernal_ = std::move(fk);
  }
  std::string                     virtual_field_name_;
  Field                           be_functioned_field_;
  std::unique_ptr<FunctionKernel> function_kernal_;
  std::string                     alias_;

  Field get_tmp_field() const
  {
    FieldMeta *fm = new FieldMeta(virtual_field_name_.c_str(), INTS, 1, 1, true, false);
    return Field(nullptr, fm);
  }
};