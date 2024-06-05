#include "token.hpp"
#include <ostream>

std::string _token_names[] = {
	"Comma","SemiColon", // ','  ';'
	"LeftParen","RightParen", //'(' and ')'
	"LeftBrace","RightBrace", // '{' and '}'
	"LeftBracket","RightBracket", // '[' and ']'

	//opeartors
	"Plus","Minus","Mul","Div","Mod", // +,-,*,/,%
	"Not","NotEqual", // !, !=
	"Greater","Less", // > <
	"GreaterEqual","LessEqual", // >= <=
	"Equal","EqualEqual", // = ==
	"And","Or", // && || 

	//Keywords
	"Const",
	"Int","Float","Void",
	"If","Else","While","Break","Continue","Return",

	//Others
	"LexError", //Meet an error
	"Eof",
	"Ident",
	"StringLiteral",
	"IntLiteral","FloatLiteral"
};

std::ostream& operator<<(std::ostream& os,TokenType t){
	os << _token_names[t];
    return os;
}

Token::~Token(){
    switch (this->type) {
    case IntLiteral : delete (int*)this->literal;break;
    case FloatLiteral : delete (float*)this->literal;break;
    case StringLiteral :
    case LexError:
    case Ident : delete (std::string*)this->literal;break;
    default:break;
    }
}

std::ostream& operator<<(std::ostream& os,const Token &token){
    os << '(';
    os << token.type << ',';
    switch (token.type) {
    case IntLiteral : os << *(int*)token.literal << ",";break;
    case FloatLiteral : os << *(float*)token.literal << ",";break;
    case StringLiteral :
    case LexError:
    case Ident: os << *(std::string*)token.literal << ",";break;

    default:break;
    }
    os << "line:" << token.line << ",column:" << token.column << ")";
    return os;
}

