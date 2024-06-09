#include "static_checker.hpp"
#include "syntax_tree.hpp"
#include "token.hpp"
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

ostream& operator<<(ostream& os,const VarType& vt){
    if(vt.is_const) os << "const ";
    os << vt.basetype;
    for(auto &dimen : vt.dimens) os << "[" << dimen << "]";
    os << "stepsize " << vt.stepsize << " offset : " << vt.offset;
    if(vt.is_const){
        os << " = {";
        if(vt.basetype == Int){
            for(int i = 0;i < vt.size;i += 4){
                os << *(int*)(vt.data.get()+i) << ",";
            }
        }else{
            for(int i = 0;i < vt.size;i += 4){
                os << *(float*)(vt.data.get()+vt.offset+i) << ",";
            }
        }
        os << "}";
    }
    os << endl;
    return os;
}
static_checker::static_checker() {env = new Environment;env->enclosing = nullptr;need_replace = false;}
static_checker::~static_checker() {if(env != nullptr) delete env;}

bool is_int_literal(expr* e){
    return typeid(*e) == typeid(int_literal_expr);
}
bool is_int_literal(shared_ptr<expr> e){
    return is_int_literal(e.get());
}
bool is_float_literal(expr* e){
    return typeid(*e) == typeid(float_literal_expr);
}
bool is_float_literal(shared_ptr<expr> e){
    return is_float_literal(e.get());
}
bool is_literal(expr* e){
    return typeid(*e) == typeid(int_literal_expr) || typeid(*e) == typeid(float_literal_expr);
}
bool is_literal(shared_ptr<expr> e){
    return is_literal(e.get());
}

shared_ptr<int_literal_expr> int_literal_with_vartype(int v){
    auto r = make_shared<int_literal_expr>(v);
    r->type = new VarType(true,false,Int,{});
    return r;
}
shared_ptr<float_literal_expr> float_literal_with_vartype(float v){
    auto r = make_shared<float_literal_expr>(v);
    r->type = new VarType(true,false,Float,{});
    return r;
}

// #define DEBUG

#ifdef DEBUG
ast_printerv1 debug;
#endif
void static_checker::accept(ast_node& e){}
void static_checker::accept(expr& e){}
void static_checker::accept(int_literal_expr& e){
    e.type = new VarType(true,false,Int,{});
}
void static_checker::accept(float_literal_expr& e){
    e.type = new VarType(true,false,Float,{});
}
void static_checker::accept(var_expr& e){
    if(VarType *vt = env->lookup(e.varname);vt != nullptr){
        #ifdef DEBUG
            cout << "Find \"" << e.varname << "\" : " << *vt << endl;
        #endif
        if(vt->is_const && !vt->is_array()){
            if(vt->basetype == Int){
                this->save_replace(int_literal_with_vartype(vt->int_value()));
            }else if(vt->basetype == Float){
                this->save_replace(float_literal_with_vartype(vt->float_value()));
            }
        }else{
            e.type = new VarType(*vt);
        }
    }else{
        throw e.varname + " : undefined variable.";
    }

    #ifdef DEBUG
        e.accept(debug);
        if(e.type == nullptr){return;}
        cout << (*e.type) << endl;
    #endif

}
void static_checker::accept(prefix_expr &e){
    e.rhs->accept(*this);
    check_replace(e.rhs);
    
    if(e.rhs->type->is_array()){
        throw string("Can't negative or not a array.");
    }
    if(e.rhs->type->basetype == Void){
        throw string("Can't negative or not a Void.");
    }
    if(is_literal(e.rhs)){
        if(is_float_literal(e.rhs)){
            float v = dynamic_pointer_cast<float_literal_expr>(e.rhs)->value;
            switch (e.op) {
            case Plus : this->save_replace(make_shared<float_literal_expr>(v));break;
            case Minus : this->save_replace(make_shared<float_literal_expr>(-v));break;
            case Not : this->save_replace(make_shared<float_literal_expr>(!v));break;
            default: throw string("Unreachable : Unknown prefix operator.");
            }
        }else{
            int v = dynamic_pointer_cast<int_literal_expr>(e.rhs)->value;
            switch (e.op) {
            case Plus : this->save_replace(make_shared<int_literal_expr>(v));break; 
            case Minus : this->save_replace(make_shared<int_literal_expr>(-v));break;
            case Not : this->save_replace(make_shared<int_literal_expr>(!v));break;
            default: throw string("Unreachable : Unknown prefix operator.");
            }
        }
    }else{
        VarType* rhs = e.rhs->type;
        e.type = new VarType(rhs->is_const,false,rhs->basetype,{});
    }
    #ifdef DEBUG
        e.accept(debug);
        cout << *e.type << endl;
    #endif
}



void static_checker::accept(binary_expr& e){
    e.lhs->accept(*this);
    check_replace(e.lhs);
    e.rhs->accept(*this);
    check_replace(e.rhs);
    
    if(e.lhs->type->is_array() || e.rhs->type->is_array()){
        throw string("Can't binary operate on array.");
    }
    if(e.lhs->type->basetype == Void || e.rhs->type->basetype == Void){
        throw string("Can't binary operate on void.");
    }

    TokenType expr_type;
    if(e.lhs->type->is_float() || e.rhs->type->is_float()){
        expr_type = Float;
    }else{
        expr_type = Int;
    }

    if(is_literal(e.lhs) && is_literal(e.rhs)){
        if(is_int_literal(e.lhs) && is_int_literal(e.rhs)){
            int lhs = dynamic_pointer_cast<int_literal_expr>(e.lhs)->value;
            int rhs = dynamic_pointer_cast<int_literal_expr>(e.rhs)->value;
            int res;
            switch (e.op) {
            case Plus: res = lhs+rhs;break;
            case Minus: res = lhs-rhs;break;
            case Mul : res = lhs*rhs;break;
            case Div : {
                if(rhs == 0) throw string("Can't Divide 0.");
                res = lhs/rhs;
            }break;
            case Mod: {
                if(rhs == 0) throw string("Can't Mod 0.");
                res = lhs % rhs;
            }break;
            case Or : res = lhs || rhs;break;
            case And : res = lhs && rhs;break;
            case EqualEqual : res = lhs == rhs;break;
            case NotEqual : res = lhs != rhs;break;
            case Greater : res = lhs > rhs;break;
            case GreaterEqual : res = lhs >= rhs;break;
            case Less : res = lhs < rhs;break;
            case LessEqual : res = lhs <= rhs;break;
            default: string("Unknown infix operator for two integers.");
            }
            this->save_replace(int_literal_with_vartype(res));
        }else{
            float lhs = is_float_literal(e.lhs) ?
                 dynamic_pointer_cast<float_literal_expr>(e.lhs)->value :
                 dynamic_pointer_cast<int_literal_expr>(e.lhs)->value;
            float rhs = is_float_literal(e.rhs) ?
                 dynamic_pointer_cast<float_literal_expr>(e.rhs)->value :
                 dynamic_pointer_cast<int_literal_expr>(e.rhs)->value;
            float res;
            switch (e.op) {
            case Plus: res = lhs+rhs;break;
            case Minus: res = lhs-rhs;break;
            case Mul : res = lhs*rhs;break;
            case Div : {
                res = lhs/rhs;
            }break;
            case Mod: {
                throw string("Can't Mod a float number!");
            }break;
            case Or : res = lhs || rhs;break;
            case And : res = lhs && rhs;break;
            case EqualEqual : res = lhs == rhs;break;
            case NotEqual : res = lhs != rhs;break;
            case Greater : res = lhs > rhs;break;
            case GreaterEqual : res = lhs >= rhs;break;
            case Less : res = lhs < rhs;break;
            case LessEqual : res = lhs <= rhs;break;
            default: string("Unknown infix operator for two float.");
            }
            this->save_replace(float_literal_with_vartype(res));
        }
    }else{
        e.type = new VarType(false,false,expr_type,{});
    }

    #ifdef DEBUG
        e.accept(debug);
        cout << *e.type << endl;
    #endif

}
void static_checker::accept(assign_expr& e){
    e.lhs->accept(*this);check_replace(e.lhs);
    e.rhs->accept(*this);check_replace(e.rhs);
    if(e.lhs->type->is_array() || e.lhs->type->is_const || !e.lhs->type->is_val){
        throw string("Can't assign a r-val or array.");
    }
    if(e.rhs->type->is_array()){
        throw string("Can't use an array to assign.");
    }
    if(e.rhs->type->basetype == Void){
        throw string("Can't assign Void to variables.");
    }

    e.type = new VarType(false,false,e.lhs->type->basetype,{});
}
void static_checker::accept(fun_call_expr& e){
    if(typeid(*e.func) != typeid(var_expr)){
        throw string("Function is not a variable?");
    }
    string funcname = dynamic_pointer_cast<var_expr>(e.func)->varname;
    auto f = this->funcs.find(funcname);
    if(f == this->funcs.end()){
        throw string("Undefined function : ") + funcname;
    }
    Func &func = f->second;
    for(auto &param : e.params) {
        param->accept(*this);
        check_replace(param);
    }

    if(e.params.size() != func.params.size()){
        throw funcname + string(" requires ") + to_string(func.params.size()) + " parameter(s),but given " + to_string(e.params.size());
    }
    for(int i = 0;i < e.params.size();++i){
        auto &formal = func.params[i].first;
        auto &actual = *e.params[i]->type;
        if(formal.dimens != actual.dimens){
            throw "When call " + func.params[i].second +string("The dimensions of formal parameter and actual parameter are different.");
        }
        if(actual.basetype == Void){
            throw string("Can't use Void as parameter.");
        }
        if(formal.is_array()){
            if(formal.basetype != actual.basetype){
                throw string("One is array of integer,one is array of float.");
            }
            for(int j = 1;j < formal.dimens.size();++j){
                if(formal.dimens[j] != actual.dimens[j]){
                    throw string("形参和实参的某一维度大小不一.");
                }
            }
        }
    }

    e.type = new VarType(false,false,func.return_type,{});

    #ifdef DEBUG
        e.accept(debug);
        cout << *e.type << endl;
    #endif
}
void static_checker::accept(index_expr& e){
    e.array->accept(*this);this->check_replace(e.array);
    e.index->accept(*this);this->check_replace(e.index);
    if(!e.array->type->is_array() || e.index->type->is_array()){
        throw string("Index error : not a correct (array,expr) pair.");
    }
    if(e.array->type->basetype == Void || e.index->type->basetype == Void){
        throw string("Index error : not a correct (array,expr) pair : use void.");
    }
    if(e.array->type->is_const && is_literal(e.index)){
        VarType* arr = e.array->type;
        int index = is_float_literal(e.index) ?
                 dynamic_pointer_cast<float_literal_expr>(e.index)->value :
                 dynamic_pointer_cast<int_literal_expr>(e.index)->value;
        if(arr->will_overflow(index)) {
            throw string("index will overflow.");
        }
        VarType res = arr->index(index);
        // cout << "---" << *e.array->type << " " << index << " " << res << "---";
        if(!res.is_array()){
            if(res.is_int()) this->save_replace(int_literal_with_vartype(res.int_value()));
            else this->save_replace(float_literal_with_vartype(res.float_value()));

            // cout << "Replace constant : " << res.float_value();
            // cout << '\n';
        }else{
            e.type = new VarType(res);
        }
    }else{
        e.type = new VarType(e.array->type->index());
    }

    // cerr << "New index : " << *e.type << endl;
    #ifdef DEBUG
        e.accept(debug);
        cout << *e.type << endl;
    #endif

}
void static_checker::accept(init_val& e){
    if(e.val){
        e.val->accept(*this);
        check_replace(e.val);
        if(e.val->type->basetype == Void) {throw string("Can't use void to init.");}
    }
    if(e.vals.size() > 0){
        for(auto &ival : e.vals) ival->accept(*this);
    }
}
void static_checker::accept(Type& e){
    for(auto &dimen : e.dimens) {
        dimen->accept(*this);
        check_replace(dimen);
        if(!is_literal(dimen)){
            throw string("Dimension in variable declaration or in function parameter must be known.");
        }
        if(is_float_literal(dimen)){
            int v = dynamic_pointer_cast<float_literal_expr>(dimen)->value;
            if(v <= 0){throw string("Dimension must be positive.");}
            dimen = int_literal_with_vartype(v);
        }
    }
}
void static_checker::accept(func_def& e){
    if(second_pass){
        this->enter_env();
        auto &func = this->funcs.find(e.name)->second;
        for(auto &param : func.params){
            this->env->vars.insert({param.second,param.first});
        }
        e.body->accept(*this);
        this->quit_env();
    }else{
        if(this->funcs.count(e.name) || this->env->vars.count(e.name)){
            throw string("Duplicated global name : ") + e.name;
        }
        this->enter_env();
        vector<pair<VarType, string>> formal_params;
        for(auto &p : e.fparams){
            if(env->vars.count(p.second)){
                throw string("Duplicated parameter name : ") + p.second + "in function definition : " + e.name ;
            }
            p.first.accept(*this);
            vector<int> dimens;
            for(auto &dimen : p.first.dimens){
                dimens.push_back(dynamic_pointer_cast<int_literal_expr>(dimen)->value);
            }
            formal_params.push_back({VarType(false,true,p.first.typ,dimens),p.second});
        }
        this->funcs.insert({e.name,Func{e.return_type,formal_params}});
        this->quit_env();
    }
}
void static_checker::accept(decl& e){
    if(env->vars.count(e.name)) {
        throw string("Redefine ") + e.name;
    }
    e.type.accept(*this);
    if(e.init != nullptr) e.init->accept(*this);
    
    TokenType basetype = e.type.typ;
    vector<int> dimens;
    for(auto &dimen : e.type.dimens){
        if(!is_literal(dimen)){throw string("Each dimension of array must be known in compile time.");}
        int d = is_int_literal(dimen) ? dynamic_pointer_cast<int_literal_expr>(dimen)->value
                                    : dynamic_pointer_cast<float_literal_expr>(dimen)->value;
        if(d <= 0) {throw string("Dimension of array must greater than 0.");}
        dimens.push_back(d);
    }
    VarType type(e.is_const,true,basetype,dimens);
    if((e.is_const || this->is_global()) && e.init != nullptr){
        auto aux_check = [](shared_ptr<expr> &e){
            if(!is_literal(e)){throw string("Global variable or constants must be initialized by value can be known in compile time.");}
        };
        if(e.init->val){
            aux_check(e.init->val);
        }else{
            for(int i = 0;i < e.init->vals.size();++i){
                aux_check(e.init->vals[i]->val);
            }
        }
    }
    if(e.is_const && e.init != nullptr){
        //For now,we assume initval can't be nested.
        auto aux_set_int = [](void* p,shared_ptr<expr> &e){
            int d = is_int_literal(e) ? dynamic_pointer_cast<int_literal_expr>(e)->value 
                                    : dynamic_pointer_cast<float_literal_expr>(e)->value;
            
            // cout << "Replace " << d ;
            *(int*)p = d;
        };
        auto aux_set_float = [](void* p,shared_ptr<expr> &e){
            float d = is_int_literal(e) ? dynamic_pointer_cast<int_literal_expr>(e)->value 
                                    : dynamic_pointer_cast<float_literal_expr>(e)->value;
            *(float*)p = d;
        };
        if(e.init->val){
            if(basetype == Int) aux_set_int(type.data.get(), e.init->val);
            else aux_set_float(type.data.get(), e.init->val);
        }else{
            for(int i = 0;i < e.init->vals.size();++i){
                if(basetype == Int) aux_set_int(type.data.get()+4*i, e.init->vals[i]->val);
                else aux_set_float(type.data.get()+4*i, e.init->vals[i]->val);
            }
        }
    }
    env->vars.insert({e.name,type});

    #ifdef DEBUG
        e.accept(debug);
        cout << type << endl;
    #endif
}
void static_checker::accept(block_item& e){
    if(e.declaration) e.declaration->accept(*this);
    if(e.statement) e.statement->accept(*this);
}
void static_checker::accept(stmt& e){
    //nothing
}
void static_checker::accept(empty_stmt& e){
    //nothing
}
void static_checker::accept(expr_stmt& e){
    e.e->accept(*this);check_replace(e.e);
}
void static_checker::accept(block_stmt& e){
    this->enter_env();
    for(auto &item : e.block) item.accept(*this);
    this->quit_env();
}
void static_checker::accept(if_stmt& e){
    e.cond->accept(*this);check_replace(e.cond);
    e.then_branch->accept(*this);
    if(e.else_branch) e.else_branch->accept(*this);
}
void static_checker::accept(while_stmt& e){
    e.cond->accept(*this);check_replace(e.cond);
    e.body->accept(*this);
}
void static_checker::accept(continue_stmt& e){
    //do nothing
}
void static_checker::accept(break_stmt& e){
    //do nothing
}
void static_checker::accept(return_stmt& e){
    e.return_value->accept(*this);check_replace(e.return_value);
    if(e.return_value->type->is_array()){
        throw string("Can't return array.");
    }
}

void static_checker::check(std::vector<CompUnit> &ast){
    this->funcs.insert({"putint",{Void,{ {VarType(false,true,Int,{}),"i"} }}});

    second_pass = false;
    for(auto &unit : ast){
        if(unit.declaration) unit.declaration->accept(*this);
    }
    for(auto &unit : ast){
        if(unit.function) unit.function->accept(*this);
    }
    second_pass = true;
    for(auto &unit : ast){
        if(unit.function) unit.function->accept(*this);
    }
}