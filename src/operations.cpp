#include "operations.h"

op_type construct_op_type(const string& op)
{
	if (op == "+") {
		return ADD;
	}
	else if (op == "-") {
		return SUB;
	}
	else if (op == "*") {
		return MUL;
	}
	else if (op == "/") {
		return DIV;
	}
	else if (op == "%") {
		return MOD;
	}

	return UNKNOWN_OP;
}

c_type construct_c_type(const string& type)
{
	if (type.find("char") != string::npos) {
		return CHAR;
	}
	else if (type.find("int") != string::npos) {
		return INT;
	}
	else if (type.find("long") != string::npos) {
		return FLOAT;
	}
	else if (type.find("double") != string::npos) {
		return DOUBLE;
	}

	return UNKNOWN_VAR;
}

const int c_type_table[] = {0, 1, 2, 1, 2, 3};

bool c_type_less(const c_type a, const c_type b)
{
	return c_type_table[a] < c_type_table[b];
}

bool c_type_greater(const c_type a, const c_type b)
{
	return c_type_table[a] > c_type_table[b];
}

operations operator+(const operations& a, const operations& b)
{
	operations o;
	o.char_ops = a.char_ops + b.char_ops;
	o.int_ops = a.int_ops + b.int_ops;
	o.float_ops = a.float_ops + b.float_ops;
	o.double_ops = a.double_ops + b.double_ops;
	return o;
}

void operations::operator=(const operations& o)
{
	char_ops = o.char_ops;
	int_ops = o.int_ops;
	float_ops = o.float_ops;
	double_ops = o.double_ops;
}

void operations::operator+=(const operations& o)
{
	char_ops += o.char_ops;
	int_ops += o.int_ops;
	float_ops += o.float_ops;
	double_ops += o.double_ops;
}

#define __SWITCH_TYPE_INCREMENT(op, type, n) \
({ \
	switch (type) { \
 		case CHAR:		char_ops.op += n; \
 					break; \
		case INT:		int_ops.op += n; \
					break; \
		case FLOAT:		float_ops.op += n; \
					break; \
		case DOUBLE:		double_ops.op += n; \
					break; \
		case UNKNOWN_VAR:	throw unknown_c_type(); \
					break; \
		default: throw unsupported_c_type(); \
	} \
})

void operations::add(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(add, type, n);
	*/
}

void operations::sub(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(sub, type, n);
	*/
}

void operations::mul(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(mul, type, n);
	*/
}

void operations::div(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(div, type, n);
	*/
}

void operations::mod(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(mod, type, n);
	*/
}

void operations::load(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(load, type, n);
	*/
}

void operations::store(const c_type type, const int n)
{
	/*
	__SWITCH_TYPE_INCREMENT(store, type, n);
	*/
}

void operations::inc(const op_type& op, const c_type type)
{
	switch (op) {
		case ADD: inc_add(type);
			  break;
		case SUB: inc_sub(type);
			  break;
		case MUL: inc_mul(type);
			  break;
		case DIV: inc_div(type);
			  break;
		case MOD: inc_mod(type);
			  break;
		case LOAD: inc_load(type);
			   break;
		case STORE: inc_store(type);
			    break;

		default: throw unknown_op_type();
	}
}

ostream& operator<<(ostream& out, const c_type& type)
{
	string str;
	switch (type) {
		case INT:		str = "INT";
					break;
		case LONG:		str = "LONG";
					break;
		case FLOAT:		str = "FLOAT";
					break;
		case DOUBLE:		str = "DOUBLE";
					break;
		case CHAR:		str = "CHAR";
					break;
		case UNKNOWN_VAR:	str = "UNKNOWN_VAR";
					break;

		default: throw unknown_c_type();
	}

	out << str;
	return out;
}

ostream& operator<<(ostream& out, const operations& ops)
{
	out	<< ops.char_ops << endl	
		<< ops.int_ops << endl	
		<< ops.float_ops << endl
		<< ops.double_ops << endl;

	return out;
}

latency_estimator::latency_estimator()
{
	// Assuming all integer types are the same.

	latency[CHAR][ADD] = 2;
	latency[CHAR][SUB] = 2;
	latency[CHAR][MUL] = 7;
	latency[CHAR][DIV] = 7;
	latency[CHAR][MOD] = 7;
	latency[CHAR][LOAD] = 2;
	latency[CHAR][STORE] = 6;

	latency[INT][ADD] = 2;
	latency[INT][SUB] = 2;
	latency[INT][MUL] = 7;
	latency[INT][DIV] = 7;
	latency[INT][MOD] = 7;
	latency[INT][LOAD] = 6;
	latency[INT][STORE] = 6;

	latency[LONG][ADD] = 2;
	latency[LONG][SUB] = 2;
	latency[LONG][MUL] = 7;
	latency[LONG][DIV] = 7;
	latency[LONG][MOD] = 7;
	latency[LONG][LOAD] = 6;
	latency[LONG][STORE] = 6;

	latency[FLOAT][ADD] = 6;
	latency[FLOAT][SUB] = 6;
	latency[FLOAT][MUL] = 6;
	latency[FLOAT][DIV] = 6; // assuming div is same as mul
	latency[FLOAT][MOD] = 6; // complete guess
	latency[FLOAT][LOAD] = 6;
	latency[FLOAT][STORE] = 6;

	latency[DOUBLE][ADD] = 13;
	latency[DOUBLE][SUB] = 13;
	latency[DOUBLE][MUL] = 13;
	latency[DOUBLE][DIV] = 13; // assuming div is same cost as mul
	latency[DOUBLE][MOD] = 13; // complete guess
	latency[DOUBLE][LOAD] = 6;
	latency[DOUBLE][STORE] = 6;
}

int latency_estimator::cycles(const int n, const op_type op, const c_type var) const
{
	return n * latency.find(var)->second.find(op)->second;
}

