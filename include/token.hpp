#ifndef token_hpp
#define token_hpp

#include <_types/_uint32_t.h>
#include <ostream>
#include <string>
#include <iostream>
enum TokenType{
	//Single-Char token
	Comma,SemiColon, // ','  ';'
	LeftParen,RightParen, //'(' and ')'
	LeftBrace,RightBrace, // '{' and '}'
	LeftBracket,RightBracket, // '[' and ']'

	//opeartors
	Plus,Minus,Mul,Div,Mod, // +,-,*,/,%
	Not,NotEqual, // !, !=
	Greater,Less, // > <
	GreaterEqual,LessEqual, // >= <=
	Equal,EqualEqual, // = ==
	And,Or, // && || 

	//Keywords
	Const,
	Int,Float,Void,
	If,Else,While,Break,Continue,Return,

	//Others
	LexError, //Meet an error
	Eof,
	Ident,
	StringLiteral,
	IntLiteral,FloatLiteral
};

std::ostream& operator<<(std::ostream&,const TokenType&);

struct Token {
	TokenType type;
	void* literal; //Pointer to a int,float,or string.
	uint32_t line;uint32_t column;
	Token(TokenType type,void* literal,uint32_t line,uint32_t column) :
		type(type),literal(literal),line(line),column(column){};
	Token(Token&& at) {
		type = at.type;
		literal = at.literal;
		line = at.line;
		column = at.column;
		at.literal = nullptr;
	};
	Token(const Token& at) : type(at.type),line(at.line),column(at.column) {
		switch (type) {
		case IntLiteral : literal = new int(*(int*)at.literal);break;
		case FloatLiteral : literal = new float(*(float*)at.literal);break;
		case StringLiteral :
		case LexError:
		case Ident : literal = new std::string(*(std::string*)at.literal);break;
		default: literal = nullptr;
		}
	};
	~Token();

	friend std::ostream& operator<<(std::ostream&,const Token&);
};



#endif