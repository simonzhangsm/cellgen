#ifndef VARIABLE_H
#define VARIABLE_H

#include <list>
#include <map>
#include <set>
#include <string>
#include <sstream>
using namespace std;

#include "utility.h"
#include "math_exprs.h"

const string pass_var = "pass";

class variable {
	string _type;
	string _name;
	string _definition;

public:
	variable() {}
	variable(const variable& c): 
		_type(c._type), _name(c._name), _definition(c._definition) 
		{}
	variable(const string& t, const string& l = "", const string& a = ""):
		_type(t), _name(l), _definition(a) 
		{}
	virtual ~variable() {}

	virtual string name() const { return _name; }
	virtual string type() const { return _type; }
	string definition() const { return _definition; }

	void definition(string s) { _definition = s; }

	virtual string declare() const { return type() + " " + name(); }
	virtual string define() const { return type() + " " + name() + "=" + definition(); }

	virtual string actual() const { return pass_var + "." + name(); }
	virtual string formal() const { return declare(); }

	bool is_non_scalar() const { return _type.find("*") != string::npos || _type.find("[") != string::npos; }
};

const variable prev("int", "prev", "0");

class const_variable: public variable {
public:
	const_variable(const string& t, const string& l, const string& a):
		variable("const " + t, l, a)
		{}

	virtual string declare() const { return type() + " " + name() + "=" + definition() + ";"; }
};
/* The Cell SPU C compiler doesn't allow const variables in static expressions. Oh well.
const const_variable buff_size("int", "buff_size", "16");
*/

const const_variable unrolled("int", "unrolled", "SPE_start + ((SPE_stop - SPE_start) / unroll_factor) * unroll_factor");
const const_variable epilogue("int", "epilogue", "unrolled + ((SPE_stop - SPE_start) % unroll_factor)");

class pound_define: public variable {
public:
	pound_define(const string& l, const string& a):
		variable("", l, a)
		{}
	virtual string define() const { return "#define " + name() + " " + definition() + "\n"; }
};
const string default_buff_size("64");

class region_variable: public variable {
	int region_num;
	int _depth; // What type of buffering? Zero means scalar; support single, double and triple.

public:
	region_variable(const string& t, const string& l, const string& a):
		variable(t, l, a), region_num(0), _depth(0) {}
	region_variable(const string& t, const string& l, const string& a, int r):
		variable(t, l, a), region_num(r), _depth(0) {}

	string unique_name() const { return name() + to_string<int>(region_num); }
	string unique_declare() const { return type() + " " + unique_name(); }

	virtual string actual() const { return pass_var + "." + unique_name(); }

	int depth() const { return _depth; }
	void depth(int d) { _depth = d; }
};

class private_variable: public region_variable {
public:
	private_variable(const string& t, const string& l, const string& a):
		region_variable(t, l, a) {}
	private_variable(const string& t, const string& l, const string& a, int r):
		region_variable(t, l, a, r) {}

	virtual string formal() const { return type() + " " + variable::name(); }
};

enum orientation_t { UNINITIALIZED, ROW, COLUMN };

class unitialized_access_orientation {};

class shared_variable: public region_variable {
	add_expr _math;
	list<string> _dimensions; // Dimensions for multidimensional array
	orientation_t _orientation;

public:
	shared_variable(const string& t, const string& l, const string& a, int r):
		region_variable(t, l, a, r), _orientation(UNINITIALIZED)
		{}
	shared_variable(const string& t, const string& l, const string& a, const list<string>& d, int r):
		region_variable(t, l, a, r), _dimensions(d), _orientation(UNINITIALIZED)
		{}

	add_expr math() const { return _math; }
	void math(add_expr m) { _math = m; }

	bool is_row() const { return _orientation == ROW; }
	void row() { _orientation = ROW; }

	bool is_column() const { return _orientation == COLUMN; }
	void column() { _orientation = COLUMN; }

	bool has_orientation() const { return _orientation != UNINITIALIZED; }
	virtual string name() const
	{
		return region_variable::name() + "_adr"; 
	}

	list<string> dimensions() const { return _dimensions; }
};

class reduction_variable: public region_variable {
public:
	reduction_variable(const string& t, const string& l, const string& a, int r):
		region_variable(t, l, a, r)
		{}

	virtual string name() const { return region_variable::name() + "_red"; }
	virtual string actual() const { return "&" + pass_var + "." + name(); }

	virtual string formal() const {  return type() + "* " + name(); }
};

class buffer_adaptor {
private:
	const region_variable* v;

public:
	buffer_adaptor(const region_variable* _v): v(_v)
	{
		assert(v);
		assert(v->depth() > 0);	
	}

	string name() const { return v->region_variable::name() + "_buf"; }
	string declare() const { return type() + "* " + name(); }
	string depth() const { return to_string(v->depth()); }
	string size() const { return name() + "_sz"; }

	string type() const
	{
		size_t star = v->type().find('*');
		size_t bracket = v->type().find('['); 

		if (bracket == string::npos && star == string::npos) {
			cerr	<< "error: variable " << v->name() 
				<< " can't be made into a buffer because it is a scalar." 
				<< endl;
			exit(1);
		}

		size_t pos = (star == string::npos) ? bracket : star;
		return v->type().substr(0, pos);
	}
};

class dma_list_adaptor {
private:
	const shared_variable* v;

public:
	dma_list_adaptor(const shared_variable* _v): v(_v)
	{
		assert(v);
		assert(v->depth() > 0);
	}

	string name() const { return v->region_variable::name() + "_lst"; }
	string name(const int i) const { return v->region_variable::name() + "_lst[" + to_string(i) + "]"; }
	string name(const string s) const { return v->region_variable::name() + "_lst[" + s + "]"; }
	string type() const { return "spe_dma_list_t"; }
	string declare() const { return type() +  " " + name() + "[" + depth() + "]"; }
	string depth() const { return to_string(v->depth()); }
};

class next_adaptor {
	const region_variable* v;
public:
	next_adaptor(const region_variable* _v): v(_v) {}
	string type() const { return "int"; }
	string name() const { return v->region_variable::name() + "_nxt"; }
	string declare() const { return type() + " " + name(); }
};

class orig_adaptor {
	const variable* v;
public:
	orig_adaptor(const region_variable* _v): v(_v) {}
	string type() const { return v->variable::type(); }
	string name() const { return v->variable::name(); }
	string declare() const { return type() + " " + name(); }
};

typedef set<region_variable*>		varset;
typedef set<private_variable*>		privset;
typedef set<shared_variable*>		sharedset;
typedef set<reduction_variable*>	reduceset;
typedef map<string, shared_variable*>	symtbl;
typedef set<string>			symset;
typedef list<string>			symlist;

#endif // VARIABLE_H

