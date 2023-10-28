#pragma once

#include "common/rc.h"
#include "sql/expr/expr_attr.h"
#include <string>
// #include "storage/field/field.h"

class Tuple;
class Value;

class FunctionKernel{
public:
	virtual ~FunctionKernel() = default;
	virtual RC do_func(const Value& in, Value& out) = 0;
	void set_meta(const char* table_name, const char* field_name);
	void set_is_const(bool b){
		is_const_  = b;
	}
	void set_has_table(bool b){
		has_table_ = b;
	}

	bool get_is_const(){
		return is_const_;
	}

	// virtual AttrInfoSqlNode get_attr_info() = 0;
	virtual ExprValueAttr get_value_attr() const = 0;

protected:
	// 不应该用FieldIdentifier，因为FieldIdentifier中的字段需要resolve校验，而FunctionKernel中的参数不需要和DB校验
	// FieldIdentifier 难受，用这个的话，再在paese_defs.h中会产生循环引用
	bool is_const_;		// 函数中的参数是常量
	// 不后期设置的话，默认都是有table的
	bool has_table_{true};	// true：select length('ssstttrrr') from table1 where id > 5;
};


class LengthFunctionKernel: public FunctionKernel{
public:
	LengthFunctionKernel(bool is_const, std::string const_val)
		: const_val_(const_val){set_is_const(is_const);}
	virtual RC do_func(const Value& in, Value& out) override;

	virtual ExprValueAttr get_value_attr() const override {
		ExprValueAttr attr;
		attr.length = 4;
		attr.type = AttrType::INTS;
		attr.nullable = true;
		return attr;
	}

private:
	std::string const_val_;
};


class RoundFunctionKernel: public FunctionKernel{
public:
	RoundFunctionKernel(bool is_const, bool is_single, int decimals, float const_val)
		: is_single_(is_single), decimals_(decimals), const_val_(const_val){ set_is_const(is_const);}
	virtual RC do_func(const Value& in, Value& out) override;
	void set_decimals(int decimals){
		decimals_ = decimals;
	}
	void set_is_single_(bool b){
		is_single_ = b;
	}

	virtual ExprValueAttr get_value_attr() const override {
		ExprValueAttr attr;
		attr.length = 4;
		attr.type = AttrType::FLOATS;
		attr.nullable = true;
		return attr;
	}

private:
	bool is_single_;
	int decimals_;
	float const_val_;
};


class FormatFunctionKernel: public FunctionKernel{
public:
	FormatFunctionKernel(bool is_const, std::string format, std::string const_val)
		: format_(format), const_val_(const_val){set_is_const(is_const);}
	virtual RC do_func(const Value& in, Value& out) override;
	void set_format(std::string format){
		format_ = format;
	}

	virtual ExprValueAttr get_value_attr() const override {
		ExprValueAttr attr;
		attr.length = 200;
		attr.type = AttrType::CHARS;
		attr.nullable = true;
		return attr;
	}

private:
RC format(const std::string& input, const std::string& formatText, std::string &output );
RC format(int input_int,const std::string& formatText,std::string &output);
	std::string format_;
	std::string const_val_;
};