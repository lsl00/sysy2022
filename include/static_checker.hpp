#ifndef static_checker_hpp
#define static_checker_hpp

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <malloc/_malloc.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "syntax_tree.hpp"
#include "token.hpp"




struct Func {
	TokenType return_type;
	std::vector<std::pair<VarType, std::string>> params;
};

struct Environment {
	std::map<std::string,VarType> vars;
	Environment *enclosing;

	Environment *enter_env(){
		Environment *env = new Environment;
		env->enclosing = this;
		return env;
	}
	VarType* lookup(std::string&& s){return lookup(s);}
	VarType* lookup(std::string& s){
		if(vars.count(s)) return &vars[s]; 
		if(enclosing) return enclosing->lookup(s);
		return nullptr;
	}
};

struct static_checker : tree_visitor {
	std::map<std::string,Func> funcs;
	Environment* env;

	bool need_replace;
	bool second_pass;
	std::shared_ptr<expr> replace_expr;
	std::shared_ptr<stmt> replace_stmt;
	
	bool is_global(){return env->enclosing == nullptr;}


	void check_replace(std::shared_ptr<expr> &origin){
		if(need_replace){
			origin = replace_expr;
			replace_expr = nullptr;
			need_replace = false;
		}
	}

	void save_replace(std::shared_ptr<expr> renew){
		replace_expr = renew;
		need_replace = true;
	}

	void enter_env(){
		env = env->enter_env();
	}
	void quit_env(){
		if(env == nullptr) return;
		Environment *old = env;
		env = env->enclosing;
		delete old;
	}
	static_checker();
	~static_checker() ;

	void check(std::vector<CompUnit> &ast);

	void accept(ast_node&);
    void accept(expr &e);
    void accept(var_expr& ve) final;
    void accept(int_literal_expr& e) final;
    void accept(float_literal_expr& e) final;
    void accept(binary_expr& e) final;
	void accept(assign_expr& e) final;
    void accept(prefix_expr& e)final;
    void accept(fun_call_expr& e) final;
    void accept(index_expr& e)final;
    void accept(init_val&)final;
    void accept(Type&)final;
    void accept(func_def&) final;
    void accept(decl&) final;
    void accept(block_item&) final;
    void accept(stmt&) final;
    void accept(empty_stmt&) final;
    void accept(expr_stmt&) final;
    void accept(block_stmt&) final;
    void accept(if_stmt&) final;
    void accept(while_stmt&) final;
    void accept(continue_stmt&) final;
    void accept(break_stmt&) final;
    void accept(return_stmt&) final;
};

#endif