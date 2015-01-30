#include "converter.h"
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

ast* converter::dup(ast* source) {
	ast* result = new ast();
	result -> set_head_symbol(source->get_head_symbol());
	result -> set_parent(source->get_parent());
	result -> set_height(source->get_height());
	result -> set_period(source->get_period());
	for (int i=0; i< source->get_degree(); i++)
		result -> add_child(dup(source->get_child(i)));
	return result;
}
/*
ast* converter::substitute(ast* source, ast* old, ast* neo) {
	ast* result = new ast();
	if (source->get_head_symbol() == old->get_head_symbol()) {
		result -> set_head_symbol(neo->get_head_symbol());	
	}
	else {
		result -> set_head_symbol(source->get_head_symbol());
	}
	for (int i=0; i<source->get_degree(); i++)
		result -> add_child(substitute(source->get_child(i), old, neo));
	return result;
}
*/

ast* converter::substitute(ast* source, symbol* old, symbol* neo) {
	ast* result = dup(source);
	result -> substitute(old, neo);
	return result;
}

bool converter::get_dreal_solutions(ast* phi, map<symbol*, symbol*>& sol, bool polarity) {
/*
#include <iostream>
#include <fstream>
using namespace std;

int main () {
  ofstream myfile;
  myfile.open ("example.txt");
  myfile << "Writing this to a file.\n";
  myfile.close();
  return 0;
}
*/
	stringstream s;
	ofstream	dreal_file;
	dreal_file.open("dreal_file.smt2");

	sol.clear();

	dreal_file << phi->print_smt2(polarity);
	dreal_file.close();
	
	system("./dReal --proof --precision 0.01 dreal_file.smt2");

	ifstream	dreal_result;
	dreal_result.open("dreal_file.smt2.proof");

	char cc;
	string cline;
	string	buffer;
	vector<string>	fields;

	getline(dreal_result, cline);

	if (cline.find("SAT") == 0) {
		printf("sat answer is returned\n");
		while (dreal_result.get(cc)) { //collect the fields
			if ((isalnum(cc)||ispunct(cc))
			 		&& cc!=',' && cc!= ':' && cc!='[' && cc!= ']')
				buffer += cc;
			else {
				if (!buffer.empty())
					fields.push_back(buffer);
				buffer.clear();
			}
		}
		//for (int i=0; i<fields.size(); i++) cout<<fields[i]<<endl;

		for(int i=0; i<fields.size(); i=i+4) {
			double a = stod(fields[i+1]);
			double b = stod(fields[i+2]);
			symbol* name = symbol_table->locate_symbol(fields[i]);
			symbol* value = num_sym(to_string(0.5*(a+b)));
			sol.insert(pair<symbol*, symbol*>(name,value));
			//	cout<<fields[i]<<"["<<fields[i+1]<<","<<fields[i+2]<<"]"<<endl;
		}

		dreal_result.close();
		return true;
	}

	dreal_result.close();
	return false;
}

void converter::simplify(ast* a) {

	for (int i=0; i<a->get_degree(); i++) {
		simplify(a -> get_child(i));
	}

	if(a->get_degree()==0) return;

	if (a->get_head_name() == "+") {
		if (a->get_child_type(0) == constant 
				&& a->get_child_type(1) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c2 = a->get_child(1)->get_value();
			double c = c1+c2;
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}
	
	else if (a->get_head_name() == "*") {
		if (a->get_child(0)->get_name() == "0"
			|| a->get_child(0)->get_name() == "0.0"
			|| a->get_child(1)->get_name() == "0"
			|| a->get_child(1)->get_name() == "0.0"){
			a->set_head_symbol(num_sym("0"));
			a->clear_children();
		}

		if (a->get_child_type(0) == constant 
				&& a->get_child_type(1) == constant)
				 {
			double c1 = a->get_child(0)->get_value();
			double c2 = a->get_child(1)->get_value();
			double c = c1*c2;
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}
	else if (a->get_head_name() == "-") {
		if (a->get_child_type(0) == constant 
				&& a->get_child_type(1) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c2 = a->get_child(1)->get_value();
			double c = c1-c2;
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}
	else if (a->get_head_name() == "/") {
		if (a->get_child_type(0) == constant 
				&& a->get_child_type(1) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c2 = a->get_child(1)->get_value();
			double c = c1/c2;
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}
	else if (a->get_head_name() == "^") {
		if (a->get_child_type(0) == constant 
				&& a->get_child_type(1) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c2 = a->get_child(1)->get_value();
			double c = ::pow(c1,c2);
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}
	else if (a->get_head_name() == "sin") {
		if (a->get_child_type(0) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c = ::sin(c1);
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}
	else if (a->get_head_name() == "cos") {
		if (a->get_child_type(0) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c = ::cos(c1);
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}	
	else if (a->get_head_name() == "tan") {
		if (a->get_child_type(0) == constant) {
			double c1 = a->get_child(0)->get_value();
			double c = ::tan(c1);
			a -> set_head_symbol(num_sym(c));
			a -> clear_children();
		}
	}	

	//return a;

}

ast* converter::partial(ast* V, symbol* x) { //change the places that need to be changed, then simplify
	ast* result;
	if (V->get_head_symbol() == x) {
		result = num("1");
	}
	else if (V->get_head_symbol()->get_stype() == variable 
				|| V->get_head_symbol()->get_stype() == parameter
				|| V->get_head_symbol()->get_stype() == constant ) {
		result = num("0");
	}
	else if ( V-> get_head_name()== "+") {
		result = add(	partial(V->get_child(0), x)
						, 
						partial(V->get_child(1), x)
					);
	}
	else if ( V-> get_head_name() == "-") {
		result = sub(	partial(V->get_child(0), x)
						, 
						partial(V->get_child(1), x)
					);
	}
	else if ( V-> get_head_name() == "*") {
			result = add(
							mul(partial(V->get_child(1), x), V->get_child(0))
							,
							mul(partial(V->get_child(0), x), V->get_child(1))
						);
	}
	else if ( V-> get_head_name() == "/") {
			result = mul(
							sub(
								mul(partial(V->get_child(0), x), V->get_child(1))
								,
								mul(partial(V->get_child(1), x), V->get_child(0))
								)
							,
							pow(V->get_child(1),num("-2"))
						);
	}
	else if ( V-> get_head_name() == "^") {
		result = mul(
						mul(V->get_child(1), pow(V->get_child(0), sub(V->get_child(1), num("1"))))
						,
						partial(V->get_child(0),x) //chain rule
					);
	}
	else if (V -> get_head_name() == "sin") {
		result = mul(
						cos(V->get_child(0)),
						partial(V->get_child(0),x)
					);
	}
	else if (V -> get_head_name() == "cos") {
		result = mul(
						sub(num("0"), sin(V->get_child(0))),
						partial(V->get_child(0),x)
					);
	}
	else if (V -> get_head_name() == "tan") {
		result = mul(
						add(num("1"), pow(tan(V->get_child(0)), num("2"))),
						partial(V->get_child(0),x)
					);
	}
	else {
		result = dup(V);
	}
	simplify(result);
	return result;
}


ast* converter::lyapunov(vector<ast*>& f, vector<ast*>& x, ast* v) {
	ast* result;
	ast* pos_v;
	ast* neg_dv = num("0");

	pos_v = geq(v, num("0"));

	for (int i=0; i< f.size(); i++) {
		ast* term = mul(partial(v, x[i]), f[i]);
		neg_dv = add(term, neg_dv);
	}

	neg_dv = leq(neg_dv, num("0"));
	result = land(pos_v, neg_dv);

	simplify(result);
	return result;
}


bool converter::cegis(ast* phi, vector<ast*>& x, 
						vector<ast*>& p, map<symbol*,symbol*>& sol) {
/*
	1. start with choosing some random values in p
	2. sub in values in p, solve for the negation of x
	3. sub in values of x, solve for p
	4. loop 2-3, but add new values (a ball around it) of x each time into constraint. 
	5. terminate whenever unsat. 
*/
	//vector<double> sol_values;
	ast* phi_temp = phi;
	sol.clear();
	for(int i=0; i<p.size(); i++) {
		double l = p[i]->get_head_symbol()->get_lower();
		double u = p[i]->get_head_symbol()->get_upper();
		//sol_values.push_back(0.5*(l+u));
		sol.insert(pair<symbol*,symbol*>(p[i]->get_head_symbol(), num_sym(0.5*(l+u))));
		phi_temp = substitute(phi_temp, p[i], num(0.5*(l+u)));
	}

	vector<ast*>	different_x_instances;
	//last bool sends negation:
	while(get_dreal_solutions(phi_temp, sol, false)) {
	cout<<"Found counterexamples with: "<<phi_temp -> print_smt2(false);
		cin.get();
		cout<<"one round"<<endl;
		//sol now keeps the counterexample for x
		phi_temp = phi;
		for(map<symbol*,symbol*>::iterator it=sol.begin(); it!=sol.end(); it++) {
			phi_temp = substitute(phi_temp, it->first, it->second);
		}
		different_x_instances.push_back(phi_temp);
		//take conjunction of all learned points
		for(int i=0; i<different_x_instances.size()-1; i++) { //no need to redo the last one
			phi_temp = land(different_x_instances[i], phi_temp);
		}

		cout<<"Find parameters with: "<<phi_temp -> print_smt2(true);

		if (!get_dreal_solutions(phi_temp, sol, true)) return false;
		//sol now keeps candidates for p
		phi_temp = phi;
		for(map<symbol*,symbol*>::iterator it=sol.begin(); it!=sol.end(); it++) {
			phi_temp = substitute(phi_temp, it->first, it->second);
		}
	}
	return true;	
}







