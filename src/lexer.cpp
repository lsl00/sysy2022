#include "lexer.hpp"
#include "token.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

bool is_digit(char c){
    return c >= '0' && c <= '9';
}
bool is_alpha(char c){
    if(c >= 'a' && c <= 'z') return true;
    if(c >= 'A' && c <= 'Z') return true;
    if(c == '_') return true;
    return false;
}

Token lexer::make_token(TokenType type, void *literal = nullptr){
    auto t =  Token(type,literal,this->line,this->beg_column);
    cur_beg = cur;
    beg_column = column;
    return t;
}
bool lexer::is_end(){
    return src[cur] == EOF;
}
char lexer::advance(){
    if(src[cur] == '\n'){
        line++;column = 1;
    }else{
        column++;
    }
    return src[cur++];
}
inline char lexer::peek(uint32_t i){
    return src[cur+i];
}

bool lexer::match(char expect){
    if(src[cur] == expect){
        advance();
        return true;
    }else{
        return false;
    }
}

Token lexer::next_token(){
    skip_white_comment();
    cur_beg = cur;beg_column = column;
    char c = advance();
    switch (c) {
    case ',' : return make_token(Comma);
    case ';' : return make_token(SemiColon);
    case '(' : return make_token(LeftParen);
    case ')' : return make_token(RightParen);
    case '[' : return make_token(LeftBracket);
    case ']' : return make_token(RightBracket);
    case '{' : return make_token(LeftBrace);
    case '}' : return make_token(RightBrace);

    case '>' :{
        if(match('=')) return make_token(GreaterEqual);
        else return make_token(Greater);
    }
    case '<' :{
        if(match('=')) return make_token(LessEqual);
        else return make_token(Less);
    }
    case '=' :{
        if(match('=')) return make_token(EqualEqual);
        else return make_token(Equal);
    }
    case '!' : {
        if(match('=')) return make_token(NotEqual);
        else return make_token(Not);
    }
    case '&' : {
        if(match('&')) return make_token(And);
        else return make_token(LexError,new std::string("Expect '&' after '&'."));
    }
    case '|' : {
        if(match('|')) return make_token(Or);
        else return make_token(LexError,new std::string("Expect '|' after '|'."));
    }
    
    case '+' : return make_token(Plus);
    case '-' : return make_token(Minus);
    case '*' : return make_token(Mul);
    case '/' : return make_token(Div);
    case '%' : return make_token(Mod);
    
    case '"' : {
        std::string res;
        while(peek(0) != '"'){
            res.push_back(parse_char('"'));
        }
        advance(); //eat '"'
        return make_token(StringLiteral,new std::string(std::move(res)));
    }
    case EOF : return make_token(Eof);
    default:{
        if(is_alpha(c)) return ident_or_keyword();
        if(is_digit(c)){
            if(c == '0' && (peek(0) == 'x' || peek(0) == 'X')){
                advance();//eat 'x' or 'X'
                return parse_hex();
            }else if(c == '0' && is_digit(peek(0))){
                return parse_oct();
            }else{
                return parse_dec(c);
            }
        }
    }
    }
    return make_token(LexError,new std::string("Unknown char"));
}
char lexer::parse_char(char surround){
    switch (peek(0)) {
    case '\\' : {
        advance(); //eat backslash
        switch (advance()) {//todo!
        case 'n' : return '\n';
        case 't' : return '\t';
        case '\"' : return '\"';
        default: return '\\';
        }
    }
    default: return advance();
    }
}
void lexer::skip_white_comment(){
    for(;;){
        switch (peek(0)) {
        case '\n':
        case '\t':
        case ' ':advance();continue;

        case '/' : {
            char next = peek(1);
            if(next == '/'){
                advance();advance();
                while(!is_end() && advance() != '\n') {}
                continue;
            }
            if(next == '*'){
                advance();advance();
                while(!is_end() && (peek(0) != '*' || peek(1) != '/')) advance();
                continue;
            }
        }break;
        default:break;
        }
        break;
    }
}

Token lexer::parse_oct(){
    int value = 0;
    for(;;){
        char c = advance();
        if(c >= '0' && c <= '7'){
            value *= 8;
            value += (c-'0');
            continue;
        }else if(c == '_'){
            continue;
        }else if(c == '8' || c == '9' || is_alpha(c)){
            return make_token(LexError,new std::string("Unexpected char while scanning an octonary number."));
        }
        return make_token(IntLiteral,new int(value));
    }
}
Token lexer::parse_hex(){
    int value = 0;
    for(;;){
        char c = advance();
        if(is_digit(c)){value *= 16;value += c - '0';continue;}
        if(c >= 'a' && c <= 'f'){value *= 16;value += c-'a'+10;continue;}
        if(c >= 'A' && c <= 'F'){value *= 16;value += c-'A'+10;continue;}
        if(c == '_') continue;
        if((c >= 'g' && c <= 'z') || (c >= 'G' && c <= 'Z')) return make_token(LexError,new std::string("Unpected char whiling scanning a hexadecimal number"));
        break;
    }
    return make_token(IntLiteral,new int(value));
}
Token lexer::parse_dec(char head){
    int iv = head - '0';float fv = 0,base = 0.1;
    bool is_float = false;
    while(1){
        char c = peek(0);
        if(is_alpha(c)){
            advance();
            if(is_float){
                fv += (c-'0')*base;
                base *= 0.1;
            }else{
                iv *= 10;
                iv += c-'0';
            }
        }else if(c == '.'){
            advance();
            if(is_float) return make_token(LexError,new std::string("Meet two '.' while scanning a floating number."));
            is_float = true;
            fv = iv;
        }else{
            break;
        }
    }
    if(is_float) return make_token(FloatLiteral,new float(fv));
    else return make_token(IntLiteral,new int(iv));
}

bool lexer::check_keyword(const char* keyword,int len){
    const char* start = src.c_str() + cur_beg;
    const char* end = src.c_str() + cur;
    if(end - start == len && memcmp(start, keyword, len) == 0) return true;
    else return false;
}


Token lexer::ident_or_keyword(){
    char head = src[cur_beg];
    while(is_alpha(peek(0)) || is_digit(peek(0))) advance();
    switch (head) {
    case 'b': if(check_keyword("break", 5)) return make_token(Break);break;
    case 'c': {
        if(check_keyword("continue", 8)) return make_token(Continue);
        if(check_keyword("const", 5)) return make_token(Const);
    }break;
    case 'e': if(check_keyword("else", 4)) return make_token(Else);break;
    case 'f': if(check_keyword("float", 5)) return make_token(Float);break;
    case 'i':{
        if(check_keyword("int", 3)) return make_token(Int);
        if(check_keyword("if", 2)) return make_token(If);
    }break;
    case 'r':if(check_keyword("return", 6)) return make_token(Return);break;
    case 'v':if(check_keyword("void", 4)) return make_token(Void);break;
    case 'w':if(check_keyword("while", 5)) return make_token(While);break;
    default: break;
    }
    return make_token(Ident,new std::string(src.c_str()+cur_beg,cur-cur_beg));
}