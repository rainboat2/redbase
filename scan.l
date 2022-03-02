%option noyywrap nounput noinput yylineno

%{
#include <stdio.h>
#include <array>
#include <string.h>
#include <map>
#include <ctype.h>
#include <iostream>

#include "redbase.h"
#include "parser.h"
#include "y.tab.h"

#define MAX_CHAR 5000
char charPool[MAX_CHAR];
int charPtr = 0;

int getKeyword(char* s);
char* getQuote(char* q, int len);

#ifndef yyerror
void yyerror(char const *s);
#endif

#define YY_USER_ACTION                                            \
  yylloc.first_line = yylloc.last_line;                           \
  yylloc.first_column = yylloc.last_column;                       \
  if (yylloc.last_line == yylineno)                               \
    yylloc.last_column += yyleng;                                 \
  else {                                                          \
    yylloc.last_line = yylineno;                                  \
    yylloc.last_column = yytext + yyleng - strrchr(yytext, '\n'); \
  }

YY_BUFFER_STATE bp = nullptr;
%}


letter      [A-Za-z]
digit       [0-9]
num         {digit}+
s_num       [+-]?{num}
literalChar [\)\(,;]

%%
[ \n\t]                         {/* ignore */}
{s_num}                         {
                                    sscanf(yytext, "%d", &yylval.ival);
                                    return INT;
                                }
{s_num}\.{num}                  {
                                    sscanf(yytext, "%f", &yylval.fval);
                                    return FLOAT;
                                }
{s_num}\.{num}[Ee]{s_num}       {
                                    sscanf(yytext, "%f", &yylval.fval);
                                    return FLOAT;
                                }
\"([^\"\n]|(\"\"))*\"           {
                                    yylval.str = getQuote(yytext, yyleng);
                                    return STRING;
                                }
\"([^\"\n]|(\"\"))*\n           { yyerror("error string!"); }
{letter}({letter}|{digit}|_)*   { return getKeyword(yylval.str = yytext); }
{literalChar}                   { return yytext[0];}
.                               { yyerror("unkown token!");}
%%

char* makeString(char* s, int len){
    if (charPtr + len > MAX_CHAR){
        std::cerr << "char pool out of memory" << std::endl;
        exit(1);
    }
    char* rs = charPool + charPtr;
    charPtr += (len + 1);
    return rs;
}

char* getQuote(char* q, int len){
    q[len - 1] = '\0';
    return makeString(q + 1, len - 2);
}

int getKeyword(char* s){
    char str[MAX_STRING_LEN];
    int len = 0;
    while (len < MAX_STRING_LEN && s[len] != '\0'){
        str[len] = tolower(s[len]);
        len++;
    }

    if (len == MAX_STRING_LEN){
        std::cerr << "keywords is too long" << std::endl;
        exit(1);
    }

    str[len] = '\0';

    struct StrLess {
        bool operator()(const char* s1, const char* s2) const { 
            return strncmp(s1, s2, MAX_STRING_LEN) < 0;
        }
    };
    static std::map<const char*, int, StrLess> strToToken = {
        {"create", CREATE},
        {"table", TABLE},
        {"int", INT_T},
        {"char", CHAR_T},
        {"float", FLOAT_T}
    };
    
    if (strToToken.find(str) == strToToken.end()){
        yylval.str = makeString(s, len);
        return STRING;
    }else{
        yylval.ival = strToToken[str];
        return yylval.ival;
    }
}

void reset_scanner(const char* query){
    charPtr = 0;

    if (bp != nullptr){
        yy_flush_buffer(bp);
        yy_delete_buffer(bp);
    }
    bp = yy_scan_bytes(const_cast<char*>(query), strlen(query));
    yy_switch_to_buffer(bp);
}