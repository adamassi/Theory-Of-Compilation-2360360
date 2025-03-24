%{
#include <stdio.h>
#include "tokens.hpp"
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
(\==)|(\!=)|(\>)|(\<)|(\>=)|(\<=)  return RELOP;
(\+|\-|\*|\/)           return BINOP;
\/\/([^\r\n])*          return COMMENT;
{letter}({letter}|{number}|0)*  return ID;
([1-9][0-9]*)|0 return NUM;
(0|([1-9][0-9]*))b return NUM_B; 
{whitespace} ;
\"({legal_escape_seq}|[^"\\\n\r])*\" return STRING;
\"({legal_escape_seq}|[^"\\])* return -2;
\"(\\.|[^"\\\n\r])*\" return -3;
.                    return -1;


%%


/* [a-zA-Z][a-zA-Z0-9]* return ID */



