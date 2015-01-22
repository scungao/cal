#include "ast.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

ast::ast(symbol* s)
	:	head_symbol(s), terminal(true), 
		degree(0), height(0), parent(NULL)
{
	try{
		flatname = s -> get_name();
	}
	catch(exception& e){
		cout<<"AST: error on base. "<<e.what()<<endl;
	}
	if (s->get_stype() == variable){
		add_variable(s);
		set_head_type(term);
	}
	if (s->get_stype() == parameter) {
		add_parameter(s);
		set_head_type(term);
	}
	else if (s->get_stype() == connective)
		set_head_type(formula);
	else if (s->get_stype() == pconnective)
		set_head_type(statement);
	else if (s->get_stype() == relation)
		set_head_type(formula); 
	//more
}

ast::ast(symbol* s, ast* l)
	:	head_symbol(s), terminal(false), degree(1), parent(NULL)
{
	flatname = s->get_name();
	children.push_back(l);
	flatname += "(";
	flatname += l->flatname;
	flatname += ")";
	height = (children[0]->height)+1;
}


ast::ast(symbol* s, ast* l1, ast* l2)
	:	head_symbol(s), terminal(false), degree(2), parent(NULL)
{
	flatname = s->get_name();

	children.push_back(l1);
	children.push_back(l2);

	flatname += "(";
	height = 0;
	for (int i=0; i<2; i++) {
		if ( children[i]->height > height ) 
			height = children[i]->height;
		flatname += children[i]->flatname;
		flatname += ",";
	}
	flatname.pop_back(); //delete trailing ","
	flatname += ")";
	height += 1;
}

ast::ast(symbol* s, ast* l1, ast* l2, ast* l3)
	:	head_symbol(s), terminal(false), degree(3), parent(NULL) 
{
	flatname = s->get_name();
	
	children.push_back(l1);
	children.push_back(l2);
	children.push_back(l3);

	flatname += "(";
	height = 0;
	for (int i=0; i<3; i++) {
		if ( children[i]->height > height ) 
			height = children[i]->height;
		flatname += children[i]->flatname;
		flatname += ",";
	}
	flatname.pop_back(); //delete trailing ","
	flatname += ")";
	height += 1;
}

void ast::add_child(ast* a){ 
	children.push_back(a); 
	a->set_parent(this); 
	set<symbol*>::iterator it;
	for(it = a->get_variables_set()->begin(); 
			it != a->get_variables_set()->end(); it++)
		add_variable(*it); 

	for(it = a->get_parameters_set()->begin(); 
			it != a->get_parameters_set()->end(); it++)
		add_parameter(*it);
}

string ast::print_prefix() {
	stringstream result;
	if (get_degree() !=0) result<<"(";
	result<<get_head_name();
	for (int i=0; i< children.size(); i++) {
		result <<" "<<children[i]->print_prefix();
	}
	if (get_degree() !=0) result<<")";
	return result.str();
}

string ast::print_smt2(bool print_params) {
/*
(set-logic QF_NRA)
(declare-fun x1 () Real)
(declare-fun x2 () Real)
(assert (<= 3.0 x1))
(assert (<= x1 3.14))
(assert (<= -7.0 x2))
(assert (<= x2 5.0))
(assert (<= (- (* 2.0 3.14159265) (* 2.0 (* x1 (arcsin (* (cos 0.797) (sin (/ 3.14159265 x1)))))))
(+ (- 0.591 (* 0.0331 x2)) (+ 0.506 1.0))))
(check-sat)
(exit)
*/
	stringstream result;
	stringstream vtemp; //keep bounds on vars
	stringstream ptemp;

	result<<"(set-logic QF_NRA)"<<endl;
	set<symbol*>::iterator it;
	for (it = variables.begin(); it!= variables.end(); it++) {
		result<<"(declare-fun "<<(*it)->get_name()<<" () Real)"<<endl;
		vtemp<<"(assert (<= "<<(*it)->get_name()<<" "<<(*it)->get_upper()<<"))"<<endl;
		vtemp<<"(assert (>= "<<(*it)->get_name()<<" "<<(*it)->get_lower()<<"))"<<endl;			
	}
	if (print_params) {
		for (it = parameters.begin(); it!= parameters.end(); it++) {
			result<<"(declare-fun "<<(*it)->get_name()<<" () Real)"<<endl;
			ptemp<<"(assert (<= "<<(*it)->get_name()<<" "<<(*it)->get_upper()<<"))"<<endl;
			ptemp<<"(assert (>= "<<(*it)->get_name()<<" "<<(*it)->get_lower()<<"))"<<endl;
		}

	}
	result<<vtemp.str()<<ptemp.str();
	result<<"(assert "<<print_prefix()<<")"<<endl;
	result<<"(check-sat)"<<endl;
	result<<"(exit)"<<endl;

	return result.str();
}



ast::~ast() 
{
}