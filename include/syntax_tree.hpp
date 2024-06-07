#ifndef syntax_hpp
#define syntax_hpp

#include "token.hpp"
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct tree_visitor;

struct ast_node {
    void *info;
    virtual void accept(tree_visitor&);
    virtual ~ast_node() ;
};


struct expr : ast_node {
    void accept(tree_visitor&);
    ~expr();
};

struct var_expr : expr {
    std::string varname;

    var_expr(std::string &s) : varname(s) {};
    ~var_expr();
    void accept(tree_visitor&);
};
struct int_literal_expr : expr {
    int value;

    int_literal_expr(int v) : value(v){};
    ~int_literal_expr();
    void accept(tree_visitor&);
};
struct float_literal_expr : expr {
    float value;
    float_literal_expr(float v) : value(v){};
    ~float_literal_expr();
    void accept(tree_visitor&);
};
struct binary_expr : expr {
    TokenType op;
    std::shared_ptr<expr> lhs,rhs;

    binary_expr(TokenType op,std::shared_ptr<expr> lhs,std::shared_ptr<expr> rhs) : op(op),lhs(lhs),rhs(rhs){};

    ~binary_expr();
    void accept(tree_visitor&);
};
struct prefix_expr : expr {
    TokenType op;
    std::shared_ptr<expr> rhs;

    prefix_expr(TokenType op,std::shared_ptr<expr> rhs) : op(op),rhs(rhs){};
    ~prefix_expr();
    void accept(tree_visitor&);
};
struct fun_call_expr : expr {
    std::shared_ptr<expr> func;
    std::vector<std::shared_ptr<expr>> params;

    fun_call_expr(std::shared_ptr<expr> func, std::vector<std::shared_ptr<expr>>& params) : func(func),params(params){};
    fun_call_expr(std::shared_ptr<expr> func, std::vector<std::shared_ptr<expr>>&& params) : func(func),params(params){};
    ~fun_call_expr();
    void accept(tree_visitor&);
};
struct index_expr : expr{
    std::shared_ptr<expr> array;
    std::shared_ptr<expr> index;

    index_expr(std::shared_ptr<expr> array,std::shared_ptr<expr> index) : array(array),index(index){};
    ~index_expr();
    void accept(tree_visitor&);
};

struct init_val;
struct Type;
struct func_def;
struct block_item;
struct decl;
struct stmt;
struct block_stmt;

struct init_val : ast_node {
    std::shared_ptr<expr> val;
    std::vector<std::shared_ptr<init_val>> vals;

    init_val(std::shared_ptr<expr> v) : val(v){};
    init_val(std::shared_ptr<expr> v,std::vector<std::shared_ptr<init_val>> &&vals) : val(v),vals(vals){};
    void accept(tree_visitor&);
};

struct Type : ast_node {
    TokenType typ; //Void Int or Float
    std::vector<std::shared_ptr<expr>> dimens;

    Type() = default;
    Type(const Type &t) = default;
    Type(const Type &&t) : typ(t.typ),dimens(std::move(t.dimens)){};
    void accept(tree_visitor&);
};

struct func_def : ast_node {
    TokenType return_type;
    std::string name;
    std::vector<std::pair<Type,std::string>> fparams;
    std::shared_ptr<block_stmt> body;

    func_def(TokenType t,std::string &n,std::vector<std::pair<Type,std::string>> &&fparams,std::shared_ptr<block_stmt> body) :
        return_type(t),name(n),fparams(fparams),body(body){}

    void accept(tree_visitor&);
};
struct decl : ast_node {
    bool is_const;
    Type type;
    std::string name;
    std::shared_ptr<init_val> init;

    decl(bool is_const,Type &&t,std::string &name,std::shared_ptr<init_val> init)
        : is_const(is_const),type(t),name(name),init(init){}
    void accept(tree_visitor&);
};

struct block_item : ast_node {
    std::shared_ptr<stmt> statement;
    std::shared_ptr<decl> declaration;

    block_item(std::shared_ptr<stmt> statement,std::shared_ptr<decl> declaration) :statement(statement),declaration(declaration){};
    void accept(tree_visitor&);
};

struct stmt : ast_node {
    void accept(tree_visitor&);
};

struct empty_stmt : stmt {
    void accept(tree_visitor&);
};
struct expr_stmt : stmt {
    std::shared_ptr<expr> e;

    expr_stmt(std::shared_ptr<expr> e) : e(e){};
    void accept(tree_visitor&);
};
struct block_stmt : stmt {
    std::vector<block_item> block;

    block_stmt(std::vector<block_item> &&block) : block(block){};
    void accept(tree_visitor&);
};
struct if_stmt : stmt {
    std::shared_ptr<expr> cond;
    std::shared_ptr<stmt> then_branch;
    std::shared_ptr<stmt> else_branch;

    if_stmt(
        std::shared_ptr<expr> cond,
        std::shared_ptr<stmt> then_branch,
        std::shared_ptr<stmt> else_branch
    ): cond(cond),then_branch(then_branch),else_branch(else_branch){}
    void accept(tree_visitor&);
};
struct while_stmt : stmt {
    std::shared_ptr<expr> cond;
    std::shared_ptr<stmt> body;
    while_stmt(std::shared_ptr<expr> cond,std::shared_ptr<stmt> body) : cond(cond),body(body){}
    void accept(tree_visitor&);
};
struct continue_stmt : stmt {
    void accept(tree_visitor&);
};
struct break_stmt : stmt{
    void accept(tree_visitor&);
};
struct return_stmt : stmt {
    std::shared_ptr<expr> return_value;

    return_stmt(std::shared_ptr<expr> e) : return_value(e){}
    void accept(tree_visitor&);
};

struct CompUnit{
    std::shared_ptr<decl> declaration;
    std::shared_ptr<func_def> function;
    void accept(tree_visitor&);
};



struct tree_visitor {
    virtual void accept(ast_node&) = 0;
    virtual void accept(expr&) = 0;
    virtual void accept(var_expr&) = 0;
    virtual void accept(int_literal_expr&) = 0;
    virtual void accept(float_literal_expr&) = 0;
    virtual void accept(binary_expr&) = 0;
    virtual void accept(prefix_expr&) = 0;
    virtual void accept(fun_call_expr&) = 0;
    virtual void accept(index_expr&) = 0;
    virtual void accept(init_val&) = 0;
    virtual void accept(Type&) = 0;
    virtual void accept(func_def&) = 0;
    virtual void accept(decl&) = 0;
    virtual void accept(block_item&) = 0;
    virtual void accept(stmt&) = 0;
    virtual void accept(empty_stmt&) = 0;
    virtual void accept(expr_stmt&) = 0;
    virtual void accept(block_stmt&) = 0;
    virtual void accept(if_stmt&) = 0;
    virtual void accept(while_stmt&) = 0;
    virtual void accept(continue_stmt&) = 0;
    virtual void accept(break_stmt&) = 0;
    virtual void accept(return_stmt&) = 0;
    void accept(CompUnit& cu){
        if(cu.declaration != nullptr){
            cu.declaration->accept(*this);
        }else if(cu.function != nullptr){
            cu.function-> accept(*this);
        }
    }
    virtual ~tree_visitor() {};
};


struct ast_printerv1 : tree_visitor {
    //print expression in S-expression.
    int blk_level;
    ast_printerv1() : blk_level(0) {};
    ~ast_printerv1() final;
    void accept(ast_node&);
    void accept(expr &e);
    void accept(var_expr& ve) final;
    void accept(int_literal_expr& e) final;
    void accept(float_literal_expr& e) final;
    void accept(binary_expr& e) final;
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