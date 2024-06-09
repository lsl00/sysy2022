#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "static_checker.hpp"
#include "syntax_tree.hpp"
using namespace std;


string read_file(const char* file){
    ifstream f(file);
    if(!f.is_open()) throw string("file doesn't exist!");
    stringstream buf;
    buf << f.rdbuf();
    f.close();
    return buf.str();
}


ostream& operator<<(ostream& os,const VarType& vt);

int main(int argc,char** argv){
    // VarType c(true,false,Int,{5,6});
    // auto d = c.index(0);
    // auto e = d.index(1);
    // cerr << e;
    if(argc == 1){
        fprintf(stderr, "Usage: %s path/to/sysy_file\n",argv[0]);
        return 1;
    }
    string src = read_file(argv[1]);
    lexer le(move(src));
    vector<Token> tokens;
    for(;;){
        Token t = le.next_token();
        tokens.push_back(t);
        if(t.type != Eof) cout << t << "\n";
        else break;
    }
    parser Parser(tokens);
    ast_printerv1 a;
    vector<CompUnit> ast;
    try{
        ast = Parser.parse();
        for(auto &cu : ast) cu.accept(a);
    }catch(string s){
        cerr << s << endl;   
    }
    static_checker checker;
    try{
        checker.check(ast);
    }catch(string s){
        cout << s << endl;
        return 0;
    }
    for(auto &cu : ast) cu.accept(a);
    return 0;
}