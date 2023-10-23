#pragma once

#include "common/rc.h"
#include "sql/parser/value.h"
#include <functional>
#include <string>

extern std::function<RC(const Value& in, Value& out)> LengthFunction;
extern std::function<RC(const Value& in, Value& out)> RoundFunction;
extern std::function<RC(const Value& in, Value& out)> FormatFunction;

class FunctionKernal{
public:
	virtual RC do_func(const Value& in, Value& out) = 0;
};

class LengthFunctionKernal: public FunctionKernal{
public:
	virtual RC do_func(const Value& in, Value& out) override;
private:
};

class RoundFunctionKernal: public FunctionKernal{
public:
	virtual RC do_func(const Value& in, Value& out) override;
	void set_decimals(int decimals){
		decimals_ = decimals;
	}
private:
	int decimals_;
};

class FormatFunctionKernal: public FunctionKernal{
public:
	virtual RC do_func(const Value& in, Value& out) override;
	void set_format(std::string format){
		format_ = format;
	}
private:
	std::string format_;
};