#ifndef parser_hpp
#define parser_hpp

#include "syntax_tree.hpp"
#include "token.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
struct parser {
    std::vector<Token> src;
    uint32_t cur;
    uint32_t line;
    uint32_t column;

    parser(std::vector<Token> &src) : src(src),cur(0),line(1),column(1){};
    parser(std::vector<Token> &&src) : src(src),cur(0),line(1),column(1){};


    bool match(TokenType type) noexcept;
    void update_line_column() noexcept;
    std::string report_line_column();
    Token& peek(uint32_t i) noexcept;
    Token& next() noexcept;
    Token& expect(TokenType type,const char* err_msg);
    std::vector<std::shared_ptr<expr>> parse_params();
    std::shared_ptr<expr> parse_expr(int min_binding);

    std::vector<CompUnit> parse();

    std::vector<std::pair<Type, std::string>> parse_fparams();
    std::shared_ptr<func_def> parse_function();
    std::vector<std::shared_ptr<decl>> parse_decl(bool);
    std::shared_ptr<init_val> parse_init();
    std::shared_ptr<block_stmt> parse_block();
    std::shared_ptr<stmt> parse_stmt();
};

#endif