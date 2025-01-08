#ifndef SEMANTIC_VISITOR_HPP
#define SEMANTIC_VISITOR_HPP

#include "visitor.hpp"
#include "output.hpp"
#include "decl.hpp"

#include <iostream>
using namespace std;
using namespace ast;


    BuiltInType getType(Node &node) {
        return BuiltInType::INT;
    }

class SemanticVisitor : public Visitor {
public:

    SemanticVisitor(output::ScopePrinter &printer) : printer(printer) {
    }

    void visit(ast::Num &node) override {
        //cout << "Num" << endl;
        //cout << "Num.type" << node.type << endl;
        // No semantic checks needed for Num
        node.type = BuiltInType::INT;
    }

    void visit(ast::NumB &node) override {
        //cout << "NumB" <<node.type << endl;
        node.type = BuiltInType::BYTE;
        if (node.value > 255) {
            output::errorByteTooLarge(node.line, node.value);
        }
    }

    void visit(ast::String &node) override {

        // No semantic checks needed for String
        // cout << "String" << endl;
        node.type = BuiltInType::STRING;
    }

    void visit(ast::Bool &node) override {
        // No semantic checks needed for Bool
        node.type = BuiltInType::BOOL;
    }

    void visit(ast::ID &node) override {
        //printer.emitVar("tetsts", ast::BuiltInType::INT, node.line);
        //std::cout << "ID" << std::endl;
        if (!symbolTable.isVariableDefined(node.value)) {
            output::errorUndef(node.line, node.value);
        }
        // added for the case when we have a assignment of a previously declared varibale
        //      this whill make sure that the type of the variable is updated.
        else{
            node.type = symbolTable.getVariableType(node.value);
        }
    }

    void visit(ast::BinOp &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        // Additional type checks can be added here
         if (node.left->type != node.right->type) {
            // i dont know if byte relop int is allowed or not
            if(node.left->type == BuiltInType::BYTE && node.right->type == BuiltInType::INT ||
                node.left->type == BuiltInType::INT && node.right->type == BuiltInType::BYTE)
            {  
                // this is legit and correct
            }
            else{
                output::errorMismatch(node.line);
            }   
        }
        node.type = ast::BuiltInType::INT;
    
    }

    void visit(ast::RelOp &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        // Additional type checks can be added here
        if (node.left->type != node.right->type) {
            // i dont know if byte relop int is allowed or not
            if(node.left->type == BuiltInType::BYTE && node.right->type == BuiltInType::INT ||
                node.left->type == BuiltInType::INT && node.right->type == BuiltInType::BYTE)
            {  
                // this is legit and correct
            }
            else{
                output::errorMismatch(node.line);
            }   
        }
        node.type = ast::BuiltInType::BOOL;
    }

    void visit(ast::Not &node) override {
        node.exp->accept(*this);
        // Additional type checks can be added here
        if(node.exp->type != BuiltInType::BOOL){
            output::errorMismatch(node.line);
        }
        node.type = ast::BuiltInType::BOOL;
    }

    void visit(ast::And &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        node.type = ast::BuiltInType::BOOL;
        // Additional type checks can be added here
    }

    void visit(ast::Or &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        node.type = ast::BuiltInType::BOOL;
        // Additional type checks can be added here
    }

    void visit(ast::Type &node) override {
       // cout << "Type " <<node.type<<BuiltInType::INT<< endl;

        // No semantic checks needed for Type
    }

    void visit(ast::Cast &node) override {
        node.exp->accept(*this);
        node.target_type->accept(*this);
        
        // Check if the type of the expression is the same as the target type
        // or if the target_type of the expression is int and the type of the expression is byte then it's legal
        // print types
        // cout << "node.exp->type::" << node.exp->type << endl;
        // cout << "node.target_type->type::" << node.target_type->type << endl;
        if (node.exp->type != node.target_type->type) {
            if (node.exp->type == BuiltInType::BYTE && node.target_type->type == BuiltInType::INT) {
                // this is legit and correct
            } else {
                if(node.exp->type == BuiltInType::INT && node.target_type->type == BuiltInType::BYTE){
                    
                    // not so sure about this, this is for test9 where the line of test is:
                    // byte b = (byte) 4; => should not return false
                    if(dynamic_cast<ast::Num*>(node.exp.get())){
                        auto num = std::dynamic_pointer_cast<ast::Num>(node.exp);
                        if(num->value > 255){
                            output::errorByteTooLarge(node.line, num->value);
                        }
                    }

                }
                else{
                    output::errorMismatch(node.line);
                }
            }
        }
        
        // Set the type of the cast expression to the target type
        node.type = node.target_type->type;
    }

    void visit(ast::ExpList &node) override {

        for (auto &exp : node.exps) {
            exp->accept(*this);
        }
    }

    void visit(ast::Call &node) override {
        //std::cout << "isFunctionDefined??  " <<node.func_id->value << std::endl; 
        if (!symbolTable.isFunctionDefined(node.func_id->value)) {
            if(symbolTable.isVariableDefined(node.func_id->value)){
                output::errorDefAsVar(node.line, node.func_id->value);
            }
                
            output::errorUndefFunc(node.line, node.func_id->value);
        }
        node.args->accept(*this);

        if (!symbolTable.logicFun(node)) {                                      
            std::vector<std::string> paramTypes = symbolTable.getParamTypesf(node.func_id->value);
            //cout << "func not defined" << endl;
            output::errorPrototypeMismatch(node.line, node.func_id->value,paramTypes);
        }

        //std::cout << "CALL "<<node.func_id->value << std::endl;
        //auto funcs =std::dynamic_pointer_cast<ast::Funcs>(node.func_id);
        //funcs->accept(*this);
        //node.func_id->accept(*this);
        //ExpList
    }

    void visit(ast::Statements &node) override {
        //cout << "Statements" << endl;
        // printer.beginScope();
        for (auto &stmt : node.statements) {
            stmt->accept(*this);
        }
        //printer.endScope();
    }

    void visit(ast::Break &node) override {
        if (!inLoop) {
            output::errorUnexpectedBreak(node.line);
        }
    }

    void visit(ast::Continue &node) override {
        if (!inLoop) {
            output::errorUnexpectedContinue(node.line);
        }
    }

    void visit(ast::Return &node) override {
        if (node.exp) {
            node.exp->accept(*this);
        }
    }

    void visit(ast::If &node) override {
        symbolTable.enterlScope();
        //printer.beginScope();
        node.condition->accept(*this);
        //
        symbolTable.enterlScope();
        printer.beginScope();
        //node.then->accept(*this);
        // if the type of the node is statement*S* so we have LBRACE and RBRACE
        // so we need to print the scope
        // Else we don't need to print the scope because we have just one statement.
        if (dynamic_cast<ast::Statements*>(node.then.get())) {
            printer.beginScope();
            node.then->accept(*this);
            printer.endScope();
        } else {
            node.then->accept(*this);
        }
        printer.endScope();
        symbolTable.exitScope();
        //
        //printer.endScope();
        symbolTable.exitScope();
        if (node.otherwise) {
            symbolTable.enterlScope();
            printer.beginScope();
            node.otherwise->accept(*this);
            printer.endScope();
            symbolTable.exitScope();
        }


        // if (node.otherwise) {
        // // Statement to be executed if the condition is false. For an if statement without else, this field is nullptr
        //     node.otherwise->accept(*this);
        // }
        // else {
        // // Statement to be executed if the condition is true
        //     node.then->accept(*this);
        // }
        //printer.endScope();
        
        
    }

    void visit(ast::While &node) override {
        //std::cout << "While" << std::endl;
        bool previousInLoop = inLoop;
        inLoop = true;
        symbolTable.enterlScope();
        printer.beginScope();
        node.condition->accept(*this);
        // symbolTable.enterlScope();
        // printer.beginScope();
        // node.body->accept(*this);
        
        // if the type of the node is statement*S* so we have LBRACE and RBRACE
        // so we need to print the scope
        // Else we don't need to print the scope because we have just one statement.
        if (dynamic_cast<ast::Statements*>(node.body.get())) {
            printer.beginScope();
            node.body->accept(*this);
            printer.endScope();
        } else {
            node.body->accept(*this);
        }
       // printer.endScope();
        //symbolTable.exitScope();
        printer.endScope();
        symbolTable.exitScope();
        //
        //node.body->accept(*this);
        inLoop = previousInLoop;
    }

    void visit(ast::VarDecl &node) override {
        //cout << "VarDecl" << endl;
        if (!symbolTable.addVariable(node.id->value, node.type->type)) {
            //std::cout << "find in VarDecl " << std::endl;

            output::errorDef(node.line, node.id->value);
        }
         node.type->accept(*this);
        // node.init_exp if the right side of the assignment is not null
        if (node.init_exp) {
            //cout << "node.init_exp" << endl;
            //han bnro7 3la numb
            node.init_exp->accept(*this);
            
            // added and works perfectally for bool values
            if (node.init_exp->type == BuiltInType::VOID) {
                if (dynamic_cast<ast::Bool*>(node.init_exp.get())) {
                    node.init_exp->type = BuiltInType::BOOL;
                }
            }
            // cout << "node.init_exp->type::" << node.init_exp->type << endl;
            // cout << "node.type->type::" << node.type->type << endl;
            if (node.init_exp->type != node.type->type) {
                if(node.init_exp->type == BuiltInType::BYTE && node.type->type == BuiltInType::INT){
                    // this is  legit and correct    
                }
                else{
                  
                    output::errorMismatch(node.line);
                  
                }
            }
            
        }
        printer.emitVar(node.id->value, node.type->type, symbolTable.getOffset(node.id->value));
    }

    void visit(ast::Assign &node) override {        
       // cout << "Assign" << endl;
        
       

        if (!symbolTable.isVariableDefined(node.id->value)) {
            output::errorUndef(node.line, node.id->value);
        }

        //  cout << "right side::" << node.exp->type << endl;
        //  cout << "left side::" << node.id->type << endl;

        node.id->accept(*this);

    
        // cout << "node assign" << endl;
        //han bnro7 3la numb
        node.exp->accept(*this);
        
        // added and works perfectally for bool values
        if (node.exp->type == BuiltInType::VOID) {
            if (dynamic_cast<ast::Bool*>(node.exp.get())) {
                node.exp->type = BuiltInType::BOOL;
            }
        }
        //  cout << "right side::" << node.exp->type << endl;
        //  cout << "left side::" << node.id->type << endl;
       
        if (node.exp->type != node.id->type) {
            if(node.exp->type == BuiltInType::BYTE && node.id->type == BuiltInType::INT){
                // this is legit and correct    
            }
            else{
                
                output::errorMismatch(node.line);
                
            }
        
        
        }
    
        
        
        // if(node.exp->type == BuiltInType::INT){
        //     cout << "byte in assign" << endl;
        // }
        //auto varType = getType(node.id->value);
        //auto exprType = getType(node.exp);


        // if (varType != exprType) {
        //     output::errorMismatch(node.line, varType, exprType);
        // }

        // bool flag_found_variable = false; 
        
        // std::vector<SymbolTable::Scope> scopes = symbolTable.scopes;
        // for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        //     if (it->variables.count(node.id->value) > 0) {
        //         // if the varibale has a type of int and the expression is type of byte
        //         if(it->variables[node.id->value].type != node.exp){ 
        //             //    allow the int variable to be assigned to byte value
        //             if(it->variables[node.id->value].type == BuiltInType::INT && getType(*node.exp) == BuiltInType::BYTE){
        //                 std::cout << "int to byte" << std::endl;
        //             }else{
        //                 output::errorMismatch(node.line);
        //             }

                    
        //         }
        //     }
            

            //  if (dynamic_cast<Cast*>(&node)) 
            //     {
            //         output::errorMismatch(node.line);
            //         if(it->variables[node.id->value].type == BuiltInType::BYTE && getType(*node.exp) == BuiltInType::INT)
            //         {
            //             output::errorMismatch(node.line);
            //         }
            //             //std::cout << "int to byte" << std::endl;
            //     }
        //}

    }

    void visit(ast::Formal &node) override {

        symbolTable.addVariablef(node.id->value, node.type->type);
            //std::cout << "find in Formal " << std::endl;
            //output::errorDef(node.line, node.id->value);
        //}
        printer.emitVar(node.id->value, node.type->type, symbolTable.getOffset(node.id->value));
    }

    void visit(ast::Formals &node) override {
        symbolTable.currentOffset = -1;
        for (auto &formal : node.formals) {
            formal->accept(*this);
        }
        symbolTable.currentOffset = 0;
    }

    void visit(ast::FuncDecl &node) override {
        
        // not so sure if needed, the check for defined function accurs in the parser now
        if (!symbolTable.isFunctionDefined(node.id->value)) {
            //std::cout << "find in FuncDecl " << std::endl;
            output::errorDef(node.line, node.id->value);
        }
        //std::cout << "funcdecl" << std::endl;
        
        //node.id->accept(*this);
        
        /////////// hda 3la a5lab lazem an5er m7lha
        printer.emitFunc(node.id->value, node.return_type->type, symbolTable.getParamTypes(node.id->value));
        symbolTable.enterScope();
        printer.beginScope();
        node.formals->accept(*this);
        node.body->accept(*this);
        printer.endScope();
        symbolTable.exitScope();
        
    }

    void visit(ast::Funcs &node) override {
        for (auto &func : node.funcs) {
            //bro7 3la funcdecl
            func->accept(*this);
        }
    }

    output::ScopePrinter &printer;
    bool inLoop = false;
};

#endif // SEMANTIC_VISITOR_HPP