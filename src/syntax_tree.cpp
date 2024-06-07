#include "syntax_tree.hpp"
#include "token.hpp"
#include <iostream>

ast_node::~ast_node() = default;
void ast_node::accept(tree_visitor &tv){
    tv.accept(*this);
}

expr::~expr() = default;
void expr::accept(tree_visitor &tv){
    tv.accept(*this);
}

var_expr::~var_expr() = default;
void var_expr::accept(tree_visitor & tv){
    tv.accept(*this);
}
int_literal_expr::~int_literal_expr() = default;
void int_literal_expr::accept(tree_visitor &tv){
    tv.accept(*this);
}
float_literal_expr::~float_literal_expr() = default;
void float_literal_expr::accept(tree_visitor &tv){
    tv.accept(*this);
}
binary_expr::~binary_expr() = default;
void binary_expr::accept(tree_visitor &tv){
    tv.accept(*this);
}
prefix_expr::~prefix_expr() = default;
void prefix_expr::accept(tree_visitor &tv){
    tv.accept(*this);
}
fun_call_expr::~fun_call_expr() = default;
void fun_call_expr::accept(tree_visitor &tv){
    tv.accept(*this);
}
index_expr::~index_expr() = default;
void index_expr::accept(tree_visitor &tv){
    tv.accept(*this);
}
void init_val::accept(tree_visitor &tv){
    tv.accept(*this);
}
void Type::accept(tree_visitor &tv){
    tv.accept(*this);
}
void func_def::accept(tree_visitor &tv){
    tv.accept(*this);
}
void decl::accept(tree_visitor &tv){
    tv.accept(*this);
}
void block_item::accept(tree_visitor &tv){
    tv.accept(*this);
}
void stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void empty_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void expr_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void block_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void if_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void while_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void continue_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void break_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}
void return_stmt::accept(tree_visitor &tv){
    tv.accept(*this);
}

void CompUnit::accept(tree_visitor &tv){
    tv.accept(*this);
}


ast_printerv1::~ast_printerv1() {};
void ast_printerv1::accept(ast_node &an){}
void ast_printerv1::accept(expr &e){}
void ast_printerv1::accept(var_expr& ve){
    std::cout << ve.varname;
}
void ast_printerv1::accept(int_literal_expr& e) {
    std::cout << e.value;
}
void ast_printerv1::accept(float_literal_expr& e){
    std::cout << e.value;
}
void ast_printerv1::accept(binary_expr& e) {
    std::cout << "("  << e.op << " ";
    e.lhs->accept(*this);
    std::cout << " ";
    e.rhs->accept(*this);
    std::cout << ")";
}
void ast_printerv1::accept(prefix_expr& e) {
    std::cout << "(" << e.op << " ";
    e.rhs->accept(*this);
    std::cout << ")";
}
void ast_printerv1::accept(fun_call_expr& e){
    std::cout << "(";
    e.func->accept(*this);
    for(auto &param : e.params){
        std::cout << ' ';
        param->accept(*this);
    }
    std::cout << ")";
}
void ast_printerv1::accept(index_expr& e){
    std::cout << "([] ";
    e.index->accept(*this);
    std::cout << " ";
    e.array->accept(*this);
    std::cout << ")";
}
void ast_printerv1::accept(init_val& iv){
    if(iv.val){
        iv.val->accept(*this);
    }
    if(iv.vals.size() > 0){
        std::cout << "{";
        for(auto val : iv.vals){
            val->accept(*this);
            std::cout << ",";
        }
        std::cout << "}";
    }
}
void ast_printerv1::accept(Type& t){
    std::cout << t.typ;
    for(auto d : t.dimens){
        std::cout << '[';
        d->accept(*this);
        std::cout << ']';
    }
}
void ast_printerv1::accept(func_def& fd){
    std::cout << fd.return_type << " " << fd.name << "(";
    for(auto &p : fd.fparams){
        p.first.accept(*this);
        std::cout << " " << p.second << ',';
    }
    std::cout << ")";
    fd.body->accept(*this);
}
void ast_printerv1::accept(decl &d) {
    if(d.is_const) std::cout << "const ";
    d.type.accept(*this);
    std::cout << " " << d.name;
    if(d.init){
        std::cout << " = ";
        d.init->accept(*this);
    }
    std::cout << ";\n";
}
void ast_printerv1::accept(block_item &d){
    for(int i = 0;i < blk_level;++i) std::cout << '\t';
    if(d.statement) d.statement->accept(*this);
    if(d.declaration) d.declaration->accept(*this);
}
void ast_printerv1::accept(stmt &s){}
void ast_printerv1::accept(empty_stmt &s){
    std::cout << ";\n";
}
void ast_printerv1::accept(expr_stmt &s){
    s.e->accept(*this);
    std::cout << ";\n";
}
void ast_printerv1::accept(block_stmt &s){
    blk_level++;
    std::cout << "{\n" ;
    for(auto &bi : s.block) bi.accept(*this);
    for(int i = 1;i < blk_level;++i) std::cout << '\t';
    std::cout << "}\n";
    blk_level--;
}
void ast_printerv1::accept(if_stmt &s){
    std::cout << "if(";
    s.cond->accept(*this);
    std::cout << ')';
    s.then_branch->accept(*this);
    if(s.else_branch){
        std::cout << "else ";
        s.else_branch->accept(*this);
    }
    std::cout << '\n';
}
void ast_printerv1::accept(while_stmt &s){
    std::cout << "while(";
    s.cond->accept(*this);
    std::cout << ')';
    s.body->accept(*this);
    std::cout << '\n';
}
void ast_printerv1::accept(continue_stmt &s){
    std::cout << "continue;\n";
}
void ast_printerv1::accept(break_stmt &s){
    std::cout << "break;\n";
}
void ast_printerv1::accept(return_stmt &s){
    std::cout << "return ";
    if(s.return_value) s.return_value->accept(*this);
    std::cout << ";\n";
}