%{
#include <stdio.h>
#include "nodes.hpp"
#include "output.hpp"
#include "parser.tab.h"

%}

/* options for line number and continue reading */ 
%option yylineno
%option noyywrap

number ([1-9])
letter ([a-zA-Z])
whitespace ([\n\r\t ])
hex_x ([0-7])
hex_y ([0-9A-Fa-f])
legal_escape_seq (\\\\|\\\"|\\n|\\r|\\t|\\0|\\x{hex_x}{hex_y})


%%

void                    return VOID;
int                     return INT;
byte                    return BYTE;
bool                    return BOOL;
and                     return AND;
or                      return OR;
not                     return NOT;
true                    return TRUE;
false                   return FALSE;
return                  return RETURN;
if                      return IF;
else                    return ELSE;
while                   return WHILE;
break                   return BREAK;
continue                return CONTINUE;
(\;)                       return SC;
(\,)                        return COMMA;
(\()                      return LPAREN;
(\))                       return RPAREN;
(\{)                       return LBRACE;
(\})                       return RBRACE;
(\=)                       return ASSIGN;
(>|<|<=|>=)                 { yylval = std::make_shared<ast::ID>(yytext); return RELOP;}
(==|!=)					    { yylval = std::make_shared<ast::ID>(yytext); return EQUALITY;}
(\+|\-)				        { yylval = std::make_shared<ast::ID>(yytext); return ADDSUB; }
(\*|\/)						{ yylval = std::make_shared<ast::ID>(yytext); return MULDIV; }
\/\/[^\r\n]*(\r|\n|\r\n)?           ; //#comment
[a-zA-Z][a-zA-Z0-9]*        {yylval =std::make_shared<ast::ID>(yytext); return ID;}
([1-9][0-9]*)|0             {yylval =std::make_shared<ast::Num>(yytext);return NUM;}
(0|([1-9][0-9]*))b          {yylval =std::make_shared<ast::NumB>(yytext);return NUM_B;}
{whitespace} ;
\"([^\n\r\"\\]|\\[rnt"\\])+\"       {yylval =std::make_shared<ast::String>(yytext);return STRING;}
.                                   {output::errorLex(yylineno); exit(1);}


%%


/* [a-zA-Z][a-zA-Z0-9]* return ID */



