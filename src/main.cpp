#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include "lexer.hpp"
using namespace std;


string read_file(const char* file){
    ifstream f(file);
    if(!f.is_open()) throw string("file doesn't exist!");
    stringstream buf;
    buf << f.rdbuf();
    f.close();
    return buf.str();
}

int main(int argc,char** argv){
    if(argc == 1){
        fprintf(stderr, "Usage: %s path/to/sysy_file\n",argv[0]);
        return 1;
    }
    string src = read_file(argv[1]);
    lexer le(move(src));
    for(;;){
        Token t = le.next_token();
        if(t.type != Eof) cout << t << "\n";
        else break;
    }
    return 0;
}