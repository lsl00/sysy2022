#include "parser.hpp"
#include "syntax_tree.hpp"
#include "token.hpp"
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


using namespace std;
void parser::update_line_column() noexcept{
    line = src[cur].line;
    column = src[cur].column;
}
bool parser::match(TokenType type) noexcept{
    if(src[cur].type == type){
        cur++;update_line_column();
        return true;
    }else{
        return false;
    }
}
Token& parser::peek(uint32_t i) noexcept {
    if(cur + i > src.size()){
        return src.back();
    }else{
        return src[cur+i];
    }
}
Token& parser::next() noexcept {
    update_line_column();
    cur++;
    return src[cur-1];
}
string parser::report_line_column(){
    stringstream ss;
    ss << "At line " << line << ", column " << column << ":";
    return ss.str();
}
Token& parser::expect(TokenType type, const char *err_msg){
    if(peek(0).type == type){
        return next();
    }
    throw report_line_column() + err_msg;
}

const int 
    PREC_ASSIGN = 1,
    PREC_OR = 3,
    PREC_AND = 5,
    PREC_EQUAL = 7,
    PREC_COMPARE = 9,
    PREC_ADD = 11,
    PREC_MUL = 13,
    PREC_UNARY = 15,
    PREC_INDEX_CALL = 17;

int prefix_binding(TokenType typ) {
    switch (typ) {
    case Minus:
    case Plus:
    case Not:
        return PREC_UNARY;
    default: return -1;
    }
}

pair<int, int> infix_binding(TokenType typ) {
    switch (typ) {
    case Equal : return make_pair(PREC_ASSIGN+1, PREC_ASSIGN);
    
    case Or : return make_pair(PREC_OR, PREC_OR+1);

    case And : return make_pair(PREC_AND, PREC_AND + 1);

    case EqualEqual:
    case NotEqual: return make_pair(PREC_EQUAL, PREC_EQUAL+1);

    case Greater:
    case GreaterEqual:
    case Less:
    case LessEqual: return make_pair(PREC_COMPARE, PREC_COMPARE+1);

    case Plus:
    case Minus : return make_pair(PREC_ADD, PREC_ADD+1);

    case Mul:
    case Div:
    case Mod: return make_pair(PREC_MUL, PREC_MUL+1);

    default:return make_pair(-1, -1);
    }
}

int postfix_binding(TokenType typ) {
    switch (typ) {
    case LeftParen:
    case LeftBracket:
        return PREC_INDEX_CALL;
    default: return -1;
    }
}


vector<shared_ptr<expr>> parser::parse_params(){
    vector<shared_ptr<expr>> res;
    while(peek(0).type != RightParen){
        res.push_back(parse_expr(0));
        if(peek(0).type == RightParen) break;
        expect(Comma, "Expect ',' between params.");
    }
    return res;
}

shared_ptr<expr> parser::parse_expr(int min_binding){
    auto next_token = next();
    shared_ptr<expr> lhs = nullptr;

    if(next_token.type == IntLiteral){
        lhs = make_shared<int_literal_expr>(*(int*)next_token.literal);
    }else if(next_token.type == FloatLiteral){
        lhs = make_shared<float_literal_expr>(*(float*)next_token.literal);
    }else if(next_token.type == Ident){
        lhs = make_shared<var_expr>(*(string*)next_token.literal);
    }else if(next_token.type == LeftParen){
        lhs = parse_expr(0);
        expect(RightParen, "Expect ')'.");
    }else if(int rb = prefix_binding(next_token.type); rb > 0){
        shared_ptr<expr> rhs = parse_expr(rb);
        lhs = make_shared<prefix_expr>(next_token.type,rhs);
    }else{
        throw report_line_column() + " expect prefix operator or variable or literal.";
    }

    while(true){
        Token& current = peek(0);

        if(int bind_power = postfix_binding(current.type);bind_power > 0){
            if(bind_power < min_binding) {
                throw string("Unreachable.");
            }

            next();
            if(current.type == LeftBracket){ //Array index
                auto rhs = parse_expr(0);
                expect(RightBracket, "Expect ']'.");
                lhs = make_shared<index_expr>(lhs,rhs);
            }else if(current.type == LeftParen){
                auto params = parse_params();
                expect(RightParen, "Expect ')' after parameters.");
                lhs = make_shared<fun_call_expr>(lhs,std::move(params));
            }else{
                throw string("Unreachable.");
            }
            continue;
        }

        if(pair<int,int> bp = infix_binding(current.type) ; bp.first > 0){
            if(bp.first < min_binding){
                break;
            }
            next();
            auto rhs = parse_expr(bp.second);
            lhs = make_shared<binary_expr>(current.type,lhs,rhs);
            continue;
        }
        break;
    }

    return lhs;
}

shared_ptr<init_val> parser::parse_init(){
    if(peek(0).type == StringLiteral) {
        vector<shared_ptr<init_val>> initvals;
        for(char c : *(string*)next().literal) {
            initvals.push_back(make_shared<init_val>(make_shared<int_literal_expr>(c)));
        }
        return make_shared<init_val>(nullptr,std::move(initvals));
    }else if(peek(0).type == LeftBrace){
        vector<shared_ptr<init_val>> initvals;
        next();//eat '{'
        while(!match(RightBrace)){
            if(peek(0).type == LeftBrace || peek(0).type == StringLiteral) initvals.push_back(parse_init());
            else initvals.push_back(make_shared<init_val>(parse_expr(0)));
            match(Comma);
        }
        return make_shared<init_val>(nullptr,std::move(initvals));
    }else{
        return make_shared<init_val>(parse_expr(0));
    }
}

vector<shared_ptr<decl>> parser::parse_decl(bool is_const){
    TokenType baseType = next().type;
    vector<shared_ptr<decl>> decls;
    do{
        string varname = *(string*)expect(Ident, "Expect identifier in variable declaration.").literal;
        Type t;t.typ = baseType;
        while(match(LeftBracket)) {
            t.dimens.push_back(parse_expr(0));
            expect(RightBracket, "Expect ']' in dimension definition.");
        }
        if(match(Equal)){
            decls.push_back(make_shared<decl>(is_const,move(t),varname,parse_init()));
        }else{
            decls.push_back(make_shared<decl>(is_const,move(t),varname,nullptr));
        }
        if(!match(Comma)) break;
    }while(peek(0).type != SemiColon);
    expect(SemiColon, "Expect ';' after variable declaration.");

    return decls;
}


vector<pair<Type, string>> parser::parse_fparams(){
    expect(LeftParen, "Expect '(' after funtion name.");
    vector<pair<Type, string>> res;

    while(!match(RightParen)){
        match(Comma);
        
        Type t;
        switch (next().type) {
        case Int: t.typ = Int;break;
        case Float: t.typ = Float;break;
        default: throw report_line_column() + "expetc type or ')' in function parameter list.";
        }

        string param_name = *(string*)expect(Ident, "expect a parameter name.").literal;
        while(match(LeftBracket)){
            if(match(RightBracket)){
                t.dimens.push_back(make_shared<int_literal_expr>(-1));
            }else{
                t.dimens.push_back(parse_expr(0));
            }
            expect(RightBracket, "Expect ']' in dimension definition.");
        }

        res.push_back(make_pair(t, param_name));
    }

    return res;
}

shared_ptr<func_def> parser::parse_function(){
    TokenType return_type = next().type;
    string func_name = *(string*)expect(Ident, "Expect function name.").literal;
    return make_shared<func_def>(
        return_type,
        func_name,
        parse_fparams(),
        parse_block()
    );
}

shared_ptr<block_stmt> parser::parse_block() {
    expect(LeftBrace, "Block should start with '{'.");
    vector<block_item> block;
    while(true){
        if(match(RightBrace)) break;
        switch(peek(0).type){
            case Const:{
                next();
                for(auto d : parse_decl(true)) block.push_back({nullptr,d});
            }break;
            case Int:
            case Float:{
                for(auto d : parse_decl(false)) block.push_back({nullptr,d});
            }break;
            case SemiColon : next();break;
            default: block.push_back({parse_stmt(),nullptr});
        }
    }
    return make_shared<block_stmt>(move(block));
}

shared_ptr<stmt> parser::parse_stmt() {
    switch (peek(0).type) {
    case Continue : {
        next();expect(SemiColon, "expect ';' after 'continue'.");
        return make_shared<continue_stmt>();
    }
    case Break : {
        next();expect(SemiColon, "expect ';' after 'break'.");
        return make_shared<break_stmt>();
    }
    case Return : {
        next();
        if(match(SemiColon)){
            return make_shared<return_stmt>(nullptr);
        }else{
            auto e = parse_expr(0);
            expect(SemiColon, "expect ';' after expression.");
            return make_shared<return_stmt>(e);
        }
    }
    case LeftBrace : {
        return parse_block();
    }
    case SemiColon : next();return make_shared<empty_stmt>();
    case If:{
        next();expect(LeftParen, "Expect '(' after 'if'.");
        auto cond = parse_expr(0);
        expect(RightParen, "Expect ')' after condition.");
        auto then_branch = parse_stmt();
        if(match(Else)){
            auto else_branch = parse_stmt();
            return make_shared<if_stmt>(cond,then_branch,else_branch);
        }else{
            return make_shared<if_stmt>(cond,then_branch,nullptr);
        }
    }
    case While:{
        next();expect(LeftParen, "Expect '(' after 'if'.");
        auto cond = parse_expr(0);
        expect(RightParen, "Expect ')' after condition.");
        auto loop_body = parse_stmt();
        return make_shared<while_stmt>(cond,loop_body);
    }
    default:{
        auto e = parse_expr(0);
        expect(SemiColon, "Expect ';' afyer experrsion in expr_stmt.");
        return make_shared<expr_stmt>(e);
    }
    }
}


vector<CompUnit> parser::parse(){
    vector<CompUnit> ast;
    while(true){
        switch (peek(0).type) {
        case Eof : return ast;
        case SemiColon : {
                next(); //eat ';'
                continue;
            }break;
        case Const : {
            next(); //eat 'const'
            switch (peek(0).type) {
            case Int:
            case Float:break;
            default: throw report_line_column() + " Expect 'int' or 'float' after const";
            }
            for(auto d : parse_decl(true)) ast.push_back(CompUnit{d,nullptr});
        }break;
        case Void: ast.push_back(CompUnit{nullptr,parse_function()});break;
        case Int:
        case Float:{
            Token& peek2 = peek(2);
            if(peek2.type == LeftParen){
                ast.push_back(CompUnit{nullptr,parse_function()});
            }else {
                for(auto d : parse_decl(false)) ast.push_back(CompUnit{d,nullptr});
            }
        }break;
        default: throw report_line_column() + "Expect variable declaration or function definition.";
        }
    }
}