#ifndef SEMANTIC_VISITOR_HPP
#define SEMANTIC_VISITOR_HPP

#include "visitor.hpp"
#include "output.hpp"
#include "decl.hpp"

#include <iostream>


class SemanticVisitor : public Visitor {
public:

    SemanticVisitor(output::ScopePrinter &printer) : printer(printer) {
    }

    void visit(ast::Num &node) override {
        
        // No semantic checks needed for Num
    }

    void visit(ast::NumB &node) override {
        if (node.value > 255) {
            output::errorByteTooLarge(node.line, node.value);
        }
    }

    void visit(ast::String &node) override {

        // No semantic checks needed for String
    }

    void visit(ast::Bool &node) override {
        // No semantic checks needed for Bool
    }

    void visit(ast::ID &node) override {
        //printer.emitVar("tetsts", ast::BuiltInType::INT, node.line);
        //std::cout << "ID" << std::endl;
        if (!symbolTable.isVariableDefined(node.value)) {
            output::errorUndef(node.line, node.value);
        }
    }

    void visit(ast::BinOp &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        // Additional type checks can be added here
    }

    void visit(ast::RelOp &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        // Additional type checks can be added here
    }

    void visit(ast::Not &node) override {
        node.exp->accept(*this);
        // Additional type checks can be added here
    }

    void visit(ast::And &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        // Additional type checks can be added here
    }

    void visit(ast::Or &node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        // Additional type checks can be added here
    }

    void visit(ast::Type &node) override {
        // No semantic checks needed for Type
    }

    void visit(ast::Cast &node) override {
        node.exp->accept(*this);
        node.target_type->accept(*this);
        // Additional type checks can be added here
    }

    void visit(ast::ExpList &node) override {
        for (auto &exp : node.exps) {
            exp->accept(*this);
        }
    }

    void visit(ast::Call &node) override {
        //std::cout << "isFunctionDefined??  " <<node.func_id->value << std::endl; 
        if (!symbolTable.isFunctionDefined(node.func_id->value)) {
            
            output::errorUndefFunc(node.line, node.func_id->value);
        }

        std::cout << "CALL "<<node.func_id->value << std::endl;
        //auto funcs =std::dynamic_pointer_cast<ast::Funcs>(node.func_id);
        //funcs->accept(*this);
        //node.func_id->accept(*this);
        //ExpList
        node.args->accept(*this);
    }

    void visit(ast::Statements &node) override {
        for (auto &stmt : node.statements) {
            stmt->accept(*this);
        }
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
        printer.beginScope();
         node.condition->accept(*this);
        printer.endScope();
        symbolTable.exitScope();
        //
        symbolTable.enterlScope();
        printer.beginScope();
        node.then->accept(*this);
        printer.endScope();
        symbolTable.exitScope();
        //
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
        symbolTable.enterlScope();
        printer.beginScope();
        node.body->accept(*this);
        printer.endScope();
        symbolTable.exitScope();
        printer.endScope();
        symbolTable.exitScope();
        //
        //node.body->accept(*this);
        inLoop = previousInLoop;
    }

    void visit(ast::VarDecl &node) override {
        if (!symbolTable.addVariable(node.id->value, node.type->type)) {
            //std::cout << "find in VarDecl " << std::endl;

            output::errorDef(node.line, node.id->value);
        }
        if (node.init_exp) {
            node.init_exp->accept(*this);
        }
        printer.emitVar(node.id->value, node.type->type, symbolTable.getOffset(node.id->value));
    }

    void visit(ast::Assign &node) override {
        node.id->accept(*this);
        node.exp->accept(*this);
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
        // if (!symbolTable.addFunction(node.id->value, node.return_type->type)) {
        //     std::cout << "find in FuncDecl " << std::endl;
        //     output::errorDef(node.line, node.id->value);
        // }
        //std::cout << "aa" << std::endl;
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
            func->accept(*this);
        }
    }

    output::ScopePrinter &printer;
    bool inLoop = false;
};

#endif // SEMANTIC_VISITOR_HPP