#include "sql_built_in_funcions.h"
#include <cmath>
#include <string>
using namespace std;

RC LengthFunctionKernal::do_func(const Value &in, Value &out)
{
  string str = in.get_string();
  out.set_int(str.size());
  return RC::SUCCESS;
};

RC RoundFunctionKernal::do_func(const Value &in, Value &out){
    if(decimals_ > 0){
        const double multiplier = std::pow(10.0, decimals_);
        float f = std::round(in.get_float() * multiplier) / multiplier;
        out.set_float(f);
    }else if(decimals_ == 0){
        float f = std::ceil(in.get_float());
        out.set_float(f);
    }else{
        const double multiplier = std::pow(10, -decimals_);
        float f = std::round(in.get_float() / multiplier) * multiplier;
        out.set_float(f);
    }
    return RC::SUCCESS;
}

RC FormatFunctionKernal::do_func(const Value &in, Value &out){
    return RC::SUCCESS;
}