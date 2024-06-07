#ifndef lexer_hpp
#define lexer_hpp

#include "token.hpp"
#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

struct lexer {
    std::string src;
    uint32_t cur_beg;uint32_t cur;
    uint32_t line;
    uint32_t beg_column;uint32_t column;
    void init(){
        cur = cur_beg = 0; line = beg_column = column = 1;
        src.push_back(EOF);
    }
    lexer(std::string&& s) : src(s) {init();}
    lexer(const std::string& s) : src(s) {init();}
    Token next_token();
    Token make_token(TokenType type,void* literal);
    bool is_end();
    char advance();
    char peek(uint32_t i);
    bool match(char expect);
    Token ident_or_keyword();
    void skip_white_comment();
    char parse_char(char surround);
    Token parse_hex();
    Token parse_oct();
    Token parse_dec(char);
    bool check_keyword(const char*,int);
};

#endif