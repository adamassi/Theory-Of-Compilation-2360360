%{
#include <iostream>
#include <stdlib.h>
#include "nodes.hpp"
#include "output.hpp"
#include "symbol_table.hpp"


// bison declarations
extern int yylineno;
extern int yylex();
extern SymbolTable symbolTable;

void yyerror(const char*);

// root of the AST, set by the parser and used by other parts of the compiler
std::shared_ptr<ast::Node> program;
using namespace ast;
using namespace std;
%}

// Define tokens

// Define precedence and associativity
%token ID
%token LBRACE
%token RBRACE
%token VOID
%token COMMA
%token SC
%token RETURN
%token IF
%token WHILE
%token BREAK
%token CONTINUE
%token INT
%token BYTE
%token BOOL
%token NUM
%token B
%token STRING
%token TRUE
%token FALSE
%right ASSIGN
%left OR
%left AND
%left EQUALITY
%left RELOP
%left ADDSUB
%left MULDIV
%right NOT
%right LPAREN LBRACE
%left RPAREN RBRACE
%left IF ELSE
%nonassoc NUM_B


%%

// While reducing the start variable, set the root of the AST
Program:  Funcs 
{ 
        //ast::print_string("1");
        program = $1; 
}

Funcs   : 
        /*epsilon*/ 
        { 
                $$ = std::make_shared<ast::Funcs>();
        }
        | FuncDecl Funcs                                              
        {
                auto func_decl= std::dynamic_pointer_cast<ast::FuncDecl>($1);
                //symbolTable.addFunctions(*func_decl);
                auto funcs = std::dynamic_pointer_cast<ast::Funcs>($2);
                funcs->push_front(std::dynamic_pointer_cast<ast::FuncDecl>($1));
                
                //functions["print"] = {ast::BuiltInType::VOID, {ast::BuiltInType::INT}};
                $$ = funcs;
        }
  
FuncDecl :  RetType  ID  LPAREN Formals RPAREN  LBRACE Statements RBRACE   
{ 
        auto id_node = std::dynamic_pointer_cast<ast::ID>($2);
        if (symbolTable.isFunctionDefined(id_node->value)) {
            output::errorDef(id_node->line, id_node->value);
        } else {
            $$ = std::make_shared<ast::FuncDecl>(
                id_node,
                std::dynamic_pointer_cast<ast::Type>($1),
                std::dynamic_pointer_cast<ast::Formals>($4),
                std::dynamic_pointer_cast<ast::Statements>($7)
            ); 
            symbolTable.addFunctions(*std::dynamic_pointer_cast<ast::FuncDecl>($$));
        }
}

RetType :    
//{ast::print_string("type");}  
        Type                                                        
        { 
                //ast::print_string("type");
                $$ = $1; 
        }
        |     VOID                                                        
        { 
                //ast::print_string("void");
                $$ = std::make_shared<ast::Type>(ast::BuiltInType::VOID); 
        }

Formals : /*epsilon*/                                                   
{ 
        //ast::print_string("Formals epsilon");
        $$ = std::make_shared<ast::Formals>(); 
}
        | FormalsList                                                   { $$ = $1; }

FormalsList :   FormalDecl                                              
{ 
                
                auto formals = std::make_shared<ast::Formals>();
                formals->push_front(std::dynamic_pointer_cast<ast::Formal>($1));
                $$ = formals;
            }
            |   FormalDecl COMMA FormalsList                            { 
                auto formals = std::dynamic_pointer_cast<ast::Formals>($3);
                formals->push_front(std::dynamic_pointer_cast<ast::Formal>($1));
                $$ = formals;
            }

FormalDecl  :   Type ID                                                 
{      
        //print_string(std::dynamic_pointer_cast<ast::ID>($2)->value);
        $$ = std::make_shared<ast::Formal>(
            std::dynamic_pointer_cast<ast::ID>($2),
            std::dynamic_pointer_cast<ast::Type>($1)
        );
}

Statements  :   Statement                                              
            { 
   
                
            $$ = std::make_shared<ast::Statements>(std::dynamic_pointer_cast<ast::Statement>($1));
       

            }
            |   Statements Statement                                    
            { 
                auto statements = std::dynamic_pointer_cast<ast::Statements>($1);
                statements->push_back(std::dynamic_pointer_cast<ast::Statement>($2));
                $$ = statements;
            }

Statement   :   LBRACE Statements RBRACE                                
            {
                    // symbolTable.enterlScope();
                    $$ = std::dynamic_pointer_cast<ast::Statement>($2);
                    //symbolTable.exitScope();
            }
            |   Type ID SC                                              
            { 
                $$ = std::make_shared<ast::VarDecl>(
                    std::dynamic_pointer_cast<ast::ID>($2),
                    std::dynamic_pointer_cast<ast::Type>($1),
                    nullptr
                ); 
            }
            |   Type ID ASSIGN Exp SC                                   
            { 
                $$ = std::make_shared<ast::VarDecl>(
                    std::dynamic_pointer_cast<ast::ID>($2),
                    std::dynamic_pointer_cast<ast::Type>($1),
                    std::dynamic_pointer_cast<ast::Exp>($4)
                ); 
            }
            |   ID ASSIGN Exp SC                                        {
                    $$ = std::make_shared<ast::Assign>(
                    std::dynamic_pointer_cast<ast::ID>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3)
                ); }
            |   Call SC                                                 { $$ = std::dynamic_pointer_cast<ast::Call>($1); }
            |   RETURN SC                                               { $$ = std::make_shared<ast::Return>(nullptr); }
            |   RETURN Exp SC                                           { $$ = std::make_shared<ast::Return>(std::dynamic_pointer_cast<ast::Exp>($2)); }
            |   IF LPAREN Exp RPAREN Statement                          
            {       
                    $$ = std::make_shared<ast::If>(
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    std::dynamic_pointer_cast<ast::Statement>($5),
                    nullptr
                ); }
            |   IF LPAREN Exp RPAREN Statement ELSE Statement           
            { $$ = std::make_shared<ast::If>(
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    std::dynamic_pointer_cast<ast::Statement>($5),
                    std::dynamic_pointer_cast<ast::Statement>($7)
                ); }
            |   WHILE LPAREN Exp RPAREN Statement                       { $$ = std::make_shared<ast::While>(
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    std::dynamic_pointer_cast<ast::Statement>($5)
                ); }
            |   BREAK SC                                                { $$ = std::make_shared<ast::Break>(); }
            |   CONTINUE SC                                             { $$ = std::make_shared<ast::Continue>(); }

Call    :   ID LPAREN ExpList RPAREN                                    { $$ = std::make_shared<ast::Call>(std::dynamic_pointer_cast<ast::ID>($1), std::dynamic_pointer_cast<ast::ExpList>($3)); }
        |   ID LPAREN RPAREN                                            { $$ = std::make_shared<ast::Call>(std::dynamic_pointer_cast<ast::ID>($1)); }

ExpList :   Exp                                                         { $$ = std::make_shared<ast::ExpList>(std::dynamic_pointer_cast<ast::Exp>($1)); }
        |   Exp COMMA ExpList                                           
        { 
                auto expList = std::dynamic_pointer_cast<ast::ExpList>($3);
                expList->push_front(std::dynamic_pointer_cast<ast::Exp>($1));
                $$ = expList;
        }

Type    :   INT                                                         
        { 
                //ast::print_string("INT");
                $$ = std::make_shared<ast::Type>(ast::BuiltInType::INT); 
        }
        |   BYTE                                                        
        { 
                $$ = std::make_shared<ast::Type>(ast::BuiltInType::BYTE); 
        }
        |   BOOL                                                        
        { 
                $$ = std::make_shared<ast::Type>(ast::BuiltInType::BOOL); 
        }

Exp         :   LPAREN Exp RPAREN                                       
    { 
    $$ = std::dynamic_pointer_cast<ast::Exp>($2);
     }
            |   Exp ADDSUB Exp                                          
            { 
                std::string op_str = std::dynamic_pointer_cast<ast::ID>($2)->value;
                ast::BinOpType op = (op_str == "+") ? ast::BinOpType::ADD : ast::BinOpType::SUB;
                $$ = std::make_shared<ast::BinOp>(
                    std::dynamic_pointer_cast<ast::Exp>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    op
                ); 
            }
            |   Exp MULDIV Exp                                          
            { 
                std::string op_str = std::dynamic_pointer_cast<ast::ID>($2)->value;
                ast::BinOpType op = (op_str == "*") ? ast::BinOpType::MUL : ast::BinOpType::DIV;
                $$ = std::make_shared<ast::BinOp>(
                    std::dynamic_pointer_cast<ast::Exp>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    op
                ); }
            |   ID                                                      
            { 
                //print_string("ID");
                $$ = std::dynamic_pointer_cast<ast::ID>($1);
            }
            |   Call                                                    { $$ = std::dynamic_pointer_cast<ast::Call>($1); }
            |   NUM                                                     { $$ = std::dynamic_pointer_cast<ast::Num>($1); }
            |   NUM_B                                                   { $$ = std::dynamic_pointer_cast<ast::NumB>($1); }
            |   STRING                                                  { $$ = std::dynamic_pointer_cast<ast::String>($1); }
            |   TRUE                                                    { $$ = std::make_shared<ast::Bool>(true); }
            |   FALSE                                                   { $$ = std::make_shared<ast::Bool>(false); }
            |   NOT Exp                                                 { $$ = std::make_shared<ast::Not>(std::dynamic_pointer_cast<ast::Exp>($2)); }
            |   Exp AND Exp                                             { $$ = std::make_shared<ast::And>(
                    std::dynamic_pointer_cast<ast::Exp>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3)
                ); }
            |   Exp OR Exp                                              { $$ = std::make_shared<ast::Or>(
                    std::dynamic_pointer_cast<ast::Exp>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3)
                ); }
            |   Exp EQUALITY Exp                                       
             { 
                std::string op_str = std::dynamic_pointer_cast<ast::ID>($2)->value;
                ast::RelOpType op = (op_str == "==") ? ast::RelOpType::EQ : ast::RelOpType::NE;
                 $$ = std::make_shared<ast::RelOp>(
                    std::dynamic_pointer_cast<ast::Exp>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    op);  }
            |   Exp RELOP Exp                                           
                {
                    std::string op_str = std::dynamic_pointer_cast<ast::ID>($2)->value;
                    ast::RelOpType op;
                    if(op_str=="<="){
                         op =ast::RelOpType::LE;
                    }
                    if(op_str=="<"){
                         op =ast::RelOpType::LT;
                    }
                    if(op_str==">="){
                         op =ast::RelOpType::GE;
                    }
                    if(op_str==">"){
                         op =ast::RelOpType::GT;
                    }
                    $$ = std::make_shared<ast::RelOp>(
                    std::dynamic_pointer_cast<ast::Exp>($1),
                    std::dynamic_pointer_cast<ast::Exp>($3),
                    op); 
                }
            |   LPAREN Type RPAREN Exp                                  
                {
                    $$ = std::make_shared<ast::Cast>(
                    std::dynamic_pointer_cast<ast::Exp>($4),
                    std::dynamic_pointer_cast<ast::Type>($2)
                ); }

%%

// Additional code

// (printf("im here");)
void yyerror(const char*){
    output::errorSyn(yylineno);
    exit(0);
}
