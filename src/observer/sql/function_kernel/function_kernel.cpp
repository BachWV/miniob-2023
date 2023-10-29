#include "function_kernel.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include <cassert>
#include <cmath>
#include <string>
#include "sql/expr/tuple.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"

#include <string>
#include <map>
#include <regex>

std::map<std::string, std::string> monthNames = {{"01", "January"},
    {"02", "February"},
    {"03", "March"},
    {"04", "April"},
    {"05", "May"},
    {"06", "June"},
    {"07", "July"},
    {"08", "August"},
    {"09", "September"},
    {"10", "October"},
    {"11", "November"},
    {"12", "December"}};

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
  if (in.is_null_value())
  {
    out = in;
    return RC::SUCCESS;
  }
  string ori = in.get_string();
  std::string formatted;
  if(is_const_){
    format(const_val_, format_, formatted);
  }else{
    format(ori, format_, formatted);
  }
  out.set_string(formatted.c_str(), formatted.size());

  return RC::SUCCESS;
}

RC FormatFunctionKernel::format(const std::string &input, const std::string &formatText, std::string &output)
{
  output = formatText;

  // Replace %Y with the full year
  output = std::regex_replace(output, std::regex("%Y"), input.substr(0, 4));

  // Replace %y with the last two digits of the year
  output = std::regex_replace(output, std::regex("%y"), input.substr(2, 2));

  // Replace %M with the full month name
  output = std::regex_replace(output, std::regex("%M"), monthNames[input.substr(5, 2)]);

  // Replace %m with the month number
  output = std::regex_replace(output, std::regex("%m"), input.substr(5, 2));

  // Replace %D with the day with English suffix
  int         day = std::stoi(input.substr(8, 2));
  std::string suffix;
  if (day >= 11 && day <= 13) {
    suffix = "th";
  } else {
    switch (day % 10) {
      case 1: suffix = "st"; break;
      case 2: suffix = "nd"; break;
      case 3: suffix = "rd"; break;
      default: suffix = "th"; break;
    }
  }
  output = std::regex_replace(output, std::regex("%D"), std::to_string(day) + suffix);

  // Replace %d with the day without suffix
  output = std::regex_replace(output, std::regex("%d"), input.substr(8, 2));

  output = std::regex_replace(output, std::regex("%"), "");

  return RC::SUCCESS;
}

RC FormatFunctionKernel::format(int input_int, const std::string &formatText, std::string &output)
{

  std::string inputStr = std::to_string(input_int);
  // Perform the date formatting as before
  output = formatText;

  // Replace %Y with the full year
  output = std::regex_replace(output, std::regex("%Y"), inputStr.substr(0, 4));

  // Replace %y with the last two digits of the year
  output = std::regex_replace(output, std::regex("%y"), inputStr.substr(2, 2));

  // Replace %M with the full month name
  output = std::regex_replace(output, std::regex("%M"), monthNames[inputStr.substr(4, 2)]);

  // Replace %m with the month number
  output = std::regex_replace(output, std::regex("%m"), inputStr.substr(4, 2));

  // Replace %D with the day with English suffix
  int         day = input_int % 100;
  std::string suffix;
  if (day >= 11 && day <= 13) {
    suffix = "th";
  } else {
    switch (day % 10) {
      case 1: suffix = "st"; break;
      case 2: suffix = "nd"; break;
      case 3: suffix = "rd"; break;
      default: suffix = "th"; break;
    }
  }
  output = std::regex_replace(output, std::regex("%D"), std::to_string(day) + suffix);

  // Replace %d with the day without suffix
  output = std::regex_replace(output, std::regex("%d"), inputStr.substr(6, 2));

  output = std::regex_replace(output, std::regex("%"), "");

  return RC::SUCCESS;
}