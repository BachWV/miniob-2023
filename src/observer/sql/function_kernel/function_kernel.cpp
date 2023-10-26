#include "function_kernel.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include <cassert>
#include <cmath>
#include <string>
#include "sql/expr/tuple.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"

using namespace std;

RC LengthFunctionKernel::do_func(const Value &in, Value &out)
{
  if (is_const_) {
    out.set_int(const_val_.size());
  } else {
    if (in.is_null_value()) {
      out = in;
    } else {
      string str = in.get_string();
      out.set_int(str.size());
    }
  }
  return RC::SUCCESS;
};

RC RoundFunctionKernel::do_func(const Value &in, Value &out)
{
  float f;
  if (is_single_) {
    if (is_const_) {
      f = std::round(const_val_);
    } else {
      if (in.is_null_value()) {
        out = in;
        return RC::SUCCESS;
      }
      f = std::round(in.get_float());
    }
  } else {
    if (is_const_) {
      if (decimals_ > 0) {
        const float multiplier = std::pow(10.0, decimals_);
        f                      = std::round(const_val_ * multiplier) / multiplier;
      } else if (decimals_ == 0) {
        float f = std::round(const_val_);
      } else {
        const float multiplier = std::pow(10, -decimals_);
        f                      = std::round(const_val_ / multiplier) * multiplier;
      }
    } else {
      if (in.is_null_value()) {
        out = in;
        return RC::SUCCESS;
      }
      if (decimals_ > 0) {
        const float multiplier = std::pow(10.0, decimals_);
        f                      = std::round(in.get_float() * multiplier) / multiplier;
      } else if (decimals_ == 0) {
        f = std::round(in.get_float());
      } else {
        const float multiplier = std::pow(10, -decimals_);
        f                      = std::round(in.get_float() / multiplier) * multiplier;
      }
    }
  }
  out.set_float(f);
  return RC::SUCCESS;
}

RC FormatFunctionKernel::do_func(const Value &in, Value &out)
{
  assert(in.attr_type() == CHARS);
  string ori = in.get_string();

  return RC::SUCCESS;
}