#ifndef CODEGEN_VISITOR_HPP
#define CODEGEN_VISITOR_HPP

#include "visitor.hpp"
#include "output.hpp"
#include "symbol_table.hpp"
#include "decl.hpp"

#include <iostream>
using namespace std;
using namespace ast;



class CodeGenVisitor : public Visitor {
public:
    output::CodeBuffer &buffer;
    SymbolTable &symbolTable;
    bool fromFunction = false;
    BuiltInType currentType; // Context variable to hold the current type

    CodeGenVisitor(output::CodeBuffer &buffer, SymbolTable &symbolTable)
        : buffer(buffer), symbolTable(symbolTable) {
        
    }

    // Load variables from stack
    std::string load_from_stack(std::string reg, unsigned int offset, std::string in) {
        std::string var = buffer.freshVar();
        // buffer << var << " = load i32, i32* " << reg << ", align 4" << std::endl;
        return var;
    }
    

    std::string convertType(BuiltInType type) {
        switch (type) {
            case BuiltInType::INT:
                return "i32";
            case BuiltInType::BYTE:
                return "i8";
            case BuiltInType::BOOL:
                return "i1";
            case BuiltInType::STRING:
                return "i8*";
            default:
                return "void";
        }
    }
    string convertTypeString(string type){
        if(type == "INT"){
            return "i32";
        }
        else if(type == "BYTE"){
            return "i8";
        }
        else if(type == "BOOL"){
            return "i1";
        }
        else if(type == "VOID"){
            return "void";
        }
        else{
            return "i32";
        }
    }
    void visit(ast::Num &node) override {
        std::string var = buffer.freshVar();
        buffer << var << " = add i32 0, " << node.value << std::endl;
        node.code = var;
        node.type = BuiltInType::INT;
    }

    void visit(ast::NumB &node) override {
         // cout << "NumB" <<node.type << endl;
        node.type = BuiltInType::BYTE;
        if (node.value > 255) {
            output::errorByteTooLarge(node.line, node.value);
        }
        std::string var = buffer.freshVar();
        buffer << var << " = add i32 0, " << node.value << std::endl;
        node.code = var;
    }

    void visit(ast::String &node) override {
        //cout << "String" << endl;
        node.type = BuiltInType::STRING;
        std::string var = buffer.freshVar();
        std::string strLabel = buffer.emitString(node.value);
        buffer << var << " = getelementptr [" << (node.value.length() + 1) << " x i8], [" << (node.value.length() + 1) << " x i8]* " << strLabel << ", i32 0, i32 0" << std::endl;
        node.code = var;
    }

    void visit(ast::Bool &node) override {
        //cout << "Bool" << endl;
        node.type = BuiltInType::BOOL;
        //cout << "Bool.type" << node.type << endl;
        std::string var = buffer.freshVar();
        buffer << var << " = add i1 0, " << (node.value ? 1 : 0) << std::endl;
        node.code = var;
    }

    void visit(ast::ID &node) override {
        //std::cout << "ID" << std::endl;
        if (!symbolTable.isVariableDefined(node.value)) {
            output::errorUndef(node.line, node.value);
        }
        else{
            node.type = symbolTable.getVariableType(node.value);
        }
        std::string var = buffer.freshVar();
        std::string type = convertType(node.type);
        if (symbolTable.isFormalParameter(node.value)) {
            buffer << var << " = load " << type << ", " << type << "* %" << node.value << "_ptr" << std::endl;
        } else {
            buffer << var << " = load " << type << ", " << type << "* %" << node.value << std::endl;
        }
        node.code = var;
    }

    void visit(ast::BinOp &node) override {
        //std::cout << "BinOp" << std::endl;
        node.left->accept(*this);
        node.right->accept(*this);
        if (node.left->type != node.right->type) {
           if(node.left->type == BuiltInType::BYTE && node.right->type == BuiltInType::INT ||
                node.left->type == BuiltInType::INT && node.right->type == BuiltInType::BYTE)
            {  
                // this is legit and correct
                node.type = BuiltInType::INT;
            }
            else{
                //cout << "binop error mismatch" << endl;
                output::errorMismatch(node.line);
            }   
        }
        else{
            node.type = node.left->type;
        }   
        std::string var = buffer.freshVar();
        switch (node.op) {
            case ast::BinOpType::ADD:
                buffer << var << " = add i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::BinOpType::SUB:
                buffer << var << " = sub i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::BinOpType::MUL:
                buffer << var << " = mul i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
                
                
            case ast::BinOpType::DIV:
            // if (node.left->type == BuiltInType::INT) {
            //     buffer << var << " = sdiv i32 " << node.left->code << ", " << node.right->code << std::endl;
            // } else {
            //     buffer << var << " = udiv i32 " << node.left->code << ", " << node.right->code << std::endl;
            // }
                std::string extendedRight = buffer.freshVar();
                buffer << extendedRight << " = icmp ne i32 " << node.right->code << ", 0" << std::endl;
                //node.right->code = extendedRight;
                buffer << "br i1 " << extendedRight << ", label "<<"%"<<"div_ok, label "<<"%"<<"div_err" << std::endl;
                buffer.emitLabel("%div_err");
                buffer << "call void @divide_by_zero_error()" << std::endl;
                buffer << "br label %div_ok" << std::endl;
                buffer.emitLabel("%div_ok");
                buffer << var << " = sdiv i32 " << node.left->code << ", " << node.right->code << std::endl;
                
        
                break;
                
        }
       
        node.code = var;
    }

    void visit(ast::RelOp &node) override {
        //cout << "RelOp" << node.op << endl; 
        node.left->accept(*this);
        node.right->accept(*this);
        if(node.left->type == BuiltInType::BOOL|| node.right->type == BuiltInType::BOOL){
            output::errorMismatch(node.line);
        }
        // Additional type checks can be added here
        if (node.left->type != node.right->type) {
            // i dont know if byte relop int is allowed or not
            if(node.left->type == BuiltInType::BYTE && node.right->type == BuiltInType::INT ||
                node.left->type == BuiltInType::INT && node.right->type == BuiltInType::BYTE)
            {  
                // this is legit and correct
            }
            else{
                //cout << "relop error mismatch" << endl;
                output::errorMismatch(node.line);
            }   
        }
        node.type = ast::BuiltInType::BOOL;
        std::string var = buffer.freshVar();
        switch (node.op) {
            case ast::RelOpType::EQ:
                buffer << var << " = icmp eq i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::RelOpType::NE:
                buffer << var << " = icmp ne i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::RelOpType::LT:
                buffer << var << " = icmp slt i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::RelOpType::GT:
                buffer << var << " = icmp sgt i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::RelOpType::LE:
                buffer << var << " = icmp sle i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
            case ast::RelOpType::GE:
                buffer << var << " = icmp sge i32 " << node.left->code << ", " << node.right->code << std::endl;
                break;
        }
        node.code = var;
    }

    void visit(ast::Not &node) override {
        node.exp->accept(*this);
        if(node.exp->type != BuiltInType::BOOL){
            //cout << "not error mismatch" << endl;
            output::errorMismatch(node.line);
        }
        node.type = ast::BuiltInType::BOOL;
        std::string var = buffer.freshVar();
        buffer << var << " = xor i1 " << node.exp->code << ", 1" << std::endl;
        node.code = var;
    }

    void visit(ast::And &node) override {
        node.left->accept(*this);
        if (node.left->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        std::string labelTrue = buffer.freshLabel();
        std::string labelFalse = buffer.freshLabel();
        std::string labelEnd = buffer.freshLabel();
        std::string var = buffer.freshVar();
        std::string old = node.t_label;
        node.t_label = labelTrue;
        node.f_label = labelEnd;
        buffer << "br i1 " << node.left->code << ", label " << labelTrue << ", label " << labelFalse << std::endl;
        buffer.emitLabel(labelTrue);
        node.right->accept(*this);
        if (node.right->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        node.type = BuiltInType::BOOL;
        
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelFalse);
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelEnd);
        if(node.right->f_label == "") {
            if(node.right->t_label == "") {
                buffer << var << " = phi i1 [ " << node.right->code << ", " << labelTrue << "], [ 0, " << labelFalse << " ]" << std::endl;
            } else {
                buffer << var << " = phi i1 [ " << node.right->code << ", " << node.right->t_label << "], [ 0, " << labelFalse << " ]" << std::endl;
            }
            //buffer << var << " = phi i1 [ " << node.right->code << ", " << labelTrue << " ], [ 0, " << labelFalse << " ]" << std::endl;
        } else {
            buffer << var << " = phi i1 [ " << node.right->code << ", " << node.right->f_label << "], [ 0, " << labelFalse  << " ]" << std::endl;
        }
        //buffer << var << " = phi i1 [ " << node.right->code << ", " << labelTrue << " ], [ 0, " << labelFalse << " ]" << std::endl;
        node.code = var;
    }

    void visit(ast::Or &node) override {
        node.left->accept(*this);
        if (node.left->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        std::string labelTrue = buffer.freshLabel();
        std::string labelFalse = buffer.freshLabel();
        std::string labelEnd = buffer.freshLabel();
        std::string var = buffer.freshVar();
        node.t_label = labelTrue;
        node.f_label = labelEnd;
        buffer << "br i1 " << node.left->code << ", label " << labelTrue << ", label " << labelFalse << std::endl;
        buffer.emitLabel(labelTrue);
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelFalse);
        node.right->accept(*this);
        if (node.right->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        node.type = BuiltInType::BOOL;
       
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelEnd);
        if(node.right->f_label == "") {
            buffer << var << " = phi i1 [ 1, " << labelTrue << " ], [ " << node.right->code << ", " << labelFalse << " ]" << std::endl;
        } else {
            buffer << var << " = phi i1 [ 1, " << labelTrue << " ], [ " << node.right->code << ", " << node.right->f_label << " ]" << std::endl;
        }
        //buffer << var << " = phi i1 [ 1, " << node.t_label << " ], [ " << node.right->code << ", " << node.right->f_label << " ]" << std::endl;
        node.code = var;
    }

    void visit(ast::Type &node) override {
        // No code generation needed for type nodes
    }

    void visit(ast::Cast &node) override {
        //cout << "Cast" << endl;
        node.exp->accept(*this);
        node.target_type->accept(*this);
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
                    //cout << "cast error mismatch" << endl;
                    output::errorMismatch(node.line);
                }
            }
        }
        node.type = node.target_type->type;
        std::string var = buffer.freshVar();
        if (node.target_type->type == ast::BuiltInType::INT && node.exp->type == ast::BuiltInType::BYTE) {
            buffer << var << " = zext i8 " << node.exp->code << " to i32" << std::endl;
        } else if (node.target_type->type == ast::BuiltInType::BYTE && node.exp->type == ast::BuiltInType::INT) {
            buffer << var << " = trunc i32 " << node.exp->code << " to i8" << std::endl;
        } else {
            var =node.exp->code;
        }
        node.code = var;
    }

    void visit(ast::ExpList &node) override {
        for (auto &exp : node.exps) {
            exp->accept(*this);
        }
    }

    void visit(ast::Call &node) override {
        //std::cout << "isFunctionDefined??  " <<node.func_id->value << std::endl;
        if (!symbolTable.isFunctionDefined(node.func_id->value)) {
            //cout << "errorDefAsFunc" << endl;
            if(symbolTable.isVariableDefined(node.func_id->value)){
                output::errorDefAsVar(node.line, node.func_id->value);
            }
                
            output::errorUndefFunc(node.line, node.func_id->value);
        }
        //node.func_id->accept(*this);
        node.args->accept(*this);
        std::string var = buffer.freshVar();
        
         std::string args;
        for (auto &arg : node.args->exps) {
            //add fun load the arguments from stack
            

            //arg->accept(*this);
            //cout << "arg->code: " << arg->code << endl;
            args +=convertType(arg->type) +" "+arg->code + ", ";

        }
        if (!args.empty()) {
            args.pop_back();
            args.pop_back();
        }
        
        if (!symbolTable.logicFun(node)) {                                      
            std::vector<std::string> paramTypes = symbolTable.getParamTypesf(node.func_id->value);
            //cout << "func not defined" << endl;
            output::errorPrototypeMismatch(node.line, node.func_id->value,paramTypes);
        }
       
        
        node.type = symbolTable.getFunctionType(node.func_id->value);
        
        std::string currentType_string = convertType(node.type);
        // if the type of the function is not void, then we need to store the return value
        if (node.type != BuiltInType::VOID) {
           buffer << var << " = call " << currentType_string << " @" << node.func_id->value << "(" << args << ")" << std::endl;
        }
        else{
            buffer << "call " << currentType_string << " @" << node.func_id->value << "(" << args << ")" << std::endl;
        }
        node.code = var;
    }

    void visit(ast::Statements &node) override {
        //cout << "Statements" << endl;
        if(fromFunction){
            fromFunction=false;
            for (auto &stmt : node.statements) {
                stmt->t = node.t;
                currentType=node.t;
                stmt->accept(*this);
            }
        }
        else{
            
            symbolTable.enterlScope();
            for (auto &stmt : node.statements){
                //cout<<"statements.t"<<node.t<<endl;
                stmt->t = node.t;
                stmt->accept(*this);
            }
            
            symbolTable.exitScope();
        }
    }

    void visit(ast::Break &node) override {
         if (!inLoop) {
            output::errorUnexpectedBreak(node.line);
        }
        buffer << "br label " << breakLabel << std::endl;
    }

    void visit(ast::Continue &node) override {
        if (!inLoop) {
            output::errorUnexpectedContinue(node.line);
        }
        buffer << "br label " << continueLabel << std::endl;
    }

    void visit(ast::Return &node) override {
        //cout << "Return" << endl;
        if (node.exp) {
            //cout << "Return exp" << endl;
            node.exp->accept(*this);
            if (currentType!=node.exp->type)
            {
                if (!(currentType == BuiltInType::INT && node.exp->type == BuiltInType::BYTE))
                {
                    cout << "error mismatch return" << endl;
                    cout << "currentType" << currentType << endl;
                    output::errorMismatch(node.line);
                }
            }
            buffer << "ret " << convertType(node.exp->type) << " " << node.exp->code << std::endl;
        } else {
            cout << "Return void" << endl;
            if (node.t != BuiltInType::VOID)
            {
                output::errorMismatch(node.line);
            }
            buffer << "ret void" << std::endl;
        }
    }

    void visit(ast::If &node) override {
        int temp=symbolTable.currentOffset;
         symbolTable.enterlScope();

        node.condition->accept(*this);
          if (node.condition->type != BuiltInType::BOOL) {
            output::errorMismatch(node.condition->line);
        }

        std::string labelTrue = buffer.freshLabel();
        std::string labelFalse = buffer.freshLabel();
        std::string labelEnd = buffer.freshLabel();
        buffer << "br i1 " << node.condition->code << ", label " << labelTrue << ", label " << labelFalse << std::endl;
        buffer.emitLabel(labelTrue);
        node.then->accept(*this);
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelFalse);
        symbolTable.exitScope();
        symbolTable.currentOffset=temp;
        if (node.otherwise) {
            node.otherwise->accept(*this);
            buffer << "br label " << labelEnd << std::endl;
        }
        // buffer << "br label %" << labelEnd << std::endl;
        buffer.emitLabel(labelEnd);
        
        
    }

    void visit(ast::While &node) override {
        std::string labelCond = buffer.freshLabel();
        std::string labelBody = buffer.freshLabel();
        std::string labelEnd = buffer.freshLabel();
        std::string oldBreakLabel = breakLabel;
        std::string oldContinueLabel = continueLabel;
        breakLabel = labelEnd;
        continueLabel = labelCond;
        buffer << "br label " << labelCond << std::endl;
        buffer.emitLabel(labelCond);
        //
        bool previousInLoop = inLoop;
        inLoop = true;
        int temp=symbolTable.currentOffset;
        symbolTable.enterlScope();
        // 
        node.condition->accept(*this);
        buffer << "br i1 " << node.condition->code << ", label " << labelBody << ", label " << labelEnd << std::endl;
        buffer.emitLabel(labelBody);
        node.body->accept(*this);
        buffer << "br label " << labelCond << std::endl;
        buffer.emitLabel(labelEnd);
        breakLabel = oldBreakLabel;
        continueLabel = oldContinueLabel;
         if (node.condition->type != BuiltInType::BOOL) {
            output::errorMismatch(node.condition->line);
        }
       
        //symbolTable.exitScope();
        
        symbolTable.exitScope();
        symbolTable.currentOffset=temp;
        //
        //node.body->accept(*this);
        inLoop = previousInLoop;
    }

    void visit(ast::VarDecl &node) override {
         node.type->accept(*this);
        // node.init_exp if the right side of the assignment is not null
        if (node.init_exp) {
            //cout << "node.init_exp" << endl;
            //han bnro7 3la numb
            
            
            
        std::string var = "%" + node.id->value;
        buffer << var << " = alloca i32" << std::endl;
        if (node.init_exp) {
            if (dynamic_cast<ast::ID*>(node.init_exp.get())) {
                //std::cout << "errorDefAsFunc" << std::endl;
                
            
                if (symbolTable.isFunctionDefined(dynamic_cast<ast::ID*>(node.init_exp.get())->value)) {
                    //std::cout << "errorDefAsFunc" << std::endl;
                    output::errorDefAsFunc(node.line, dynamic_cast<ast::ID*>(node.init_exp.get())->value);
                }
                
                if(!symbolTable.isVariableDefined(dynamic_cast<ast::ID*>(node.init_exp.get())->value)){
                    output::errorUndef(node.line, dynamic_cast<ast::ID*>(node.init_exp.get())->value);
                }

            }
            node.init_exp->accept(*this);
             if (node.init_exp->type == BuiltInType::VOID) {
                if (dynamic_cast<ast::Bool*>(node.init_exp.get())) {
                    node.init_exp->type = BuiltInType::BOOL;
                }
            }
             //cout << "node.init_exp->type::" << node.init_exp->type << endl;
             //cout << "node.type->type::" << node.type->type << endl;
            if (node.init_exp->type != node.type->type) {
                if(node.init_exp->type == BuiltInType::BYTE && node.type->type == BuiltInType::INT){
                    // this is  legit and correct    
                }
                else{
                    //cout << "vardecl error mismatch" << endl;
                    output::errorMismatch(node.line);
                  
                }
            }
            
            buffer << "store i32 " << node.init_exp->code << ", i32* " << var << std::endl;
        } else {
            buffer << "store i32 0, i32* " << var << std::endl;
        }
        
        if (!symbolTable.addVariable(node.id->value, node.type->type)) {
            output::errorDef(node.line, node.id->value);
        }
        
        //symbolTable.addVariable(node.id->value, node.type->type);
        }
    }

    void visit(ast::Assign &node) override {
        //cout << "Assign" << endl;
         if (symbolTable.isFunctionDefined(node.id->value)) {
            //std::cout << "errorDefAsFunc" << std::endl;
            output::errorDefAsFunc(node.line, node.id->value);
        }
        if (!symbolTable.isVariableDefined(node.id->value)) {
            output::errorUndef(node.line, node.id->value);
        }

        //cout << "right side::" << node.exp->type << endl;
        //cout << "left side::" << node.id->type << endl;

        node.id->accept(*this);
        if (dynamic_cast<ast::ID*>(node.exp.get())) {
                //std::cout << "errorDefAsFunc" << std::endl;
                
            
                if (symbolTable.isFunctionDefined(dynamic_cast<ast::ID*>(node.exp.get())->value)) {
                    //std::cout << "errorDefAsFunc" << std::endl;
                    output::errorDefAsFunc(node.line, dynamic_cast<ast::ID*>(node.exp.get())->value);
                }
            }
        node.exp->accept(*this);
        
        if (node.exp->type == BuiltInType::VOID) {
            if (dynamic_cast<ast::Bool*>(node.exp.get())) {
                node.exp->type = BuiltInType::BOOL;
            }
        }
        //cout << "right side::" << node.exp->type << endl;
        //cout << "left side::" << node.id->type << endl;
       
        if (node.exp->type != node.id->type) {
            if(node.exp->type == BuiltInType::BYTE && node.id->type == BuiltInType::INT){
                // this is legit and correct    
            }
            else{
                
                output::errorMismatch(node.line);
                
            }
        
        
        }
        std::string var = "%" + node.id->value;
        buffer << "store i32 " << node.exp->code << ", i32* " << var << std::endl;
    }

    void visit(ast::Formal &node) override {
        // No code generation needed for formal parameters
         if(!symbolTable.addVariablef(node.id->value, node.type->type)){
            //cout << "error" << endl;
            output::errorDef(node.line, node.id->value);
        }
        
    }

    void visit(ast::Formals &node) override {
        // No code generation needed for formals
        symbolTable.currentOffset = -1;
        for (auto &formal : node.formals) { 
            formal->accept(*this);
        }
        symbolTable.currentOffset = 0;
    }

    void visit(ast::FuncDecl &node) override {
        //cout << "FuncDecl" << endl;
        //cout << "FuncDecl: " << node.id->value << endl;
        // not so sure if needed, the check for defined function accurs in the parser now
        if (!symbolTable.isFunctionDefined(node.id->value)) {
            //std::cout << "find in FuncDecl " << std::endl;
            output::errorDef(node.line, node.id->value);
        }
        // if(symbolTable.checkParamNames(node)){
        //     //cout << "error" << endl;
        //     output::errorDefAsFunc(node.line, node.id->value);
        // }
        //cout << "funcdecl " <<node.id->value<< std::endl;
        fromFunction=true;
        std::string currentType_string = convertType(node.return_type->type); 
        


        std::vector<std::string> paramTypes = symbolTable.getParamTypesf(node.id->value);
        int i=0;
        buffer << "define " << currentType_string << " @" << node.id->value << "(";
        for (auto &formal : node.formals->formals) {
            if(i == 0)
            {
                buffer << convertTypeString(paramTypes[i]) << " %" << formal->id->value;
            }
            else{
                buffer << ", " << convertTypeString(paramTypes[i]) << " %" << formal->id->value;
            }
            
            i++;
        }
        i=0;
        // if (!node.formals->formals.empty()) {
        //     buffer.seekp(-2, std::ios_base::end);
        // }
        buffer << ") {" << std::endl;
        symbolTable.enterScope();
        
        node.formals->accept(*this);
        node.body->t = node.return_type->type;

        for (auto &formal : node.formals->formals) {
            std::string var = "%" + formal->id->value;
            std::string type = convertType(formal->type->type);
            buffer << var << "_ptr = alloca " << type << std::endl;
            buffer << "store " << type << " %" << formal->id->value << ", " << type << "* " << var << "_ptr" << std::endl;
            symbolTable.addVariable(formal->id->value, formal->type->type);
        }
        
        //cout << "body" << endl;
        node.body->accept(*this);
        if (node.return_type->type == BuiltInType::VOID) {
            buffer << "ret void" << std::endl;
        }
        buffer << "}" << std::endl;
        if (node.return_type->type != node.body->t) {
            cout << "error mismatch" << endl;
            output::errorMismatch(node.line);
        }
        //
        
        
       // buffer << "ret " << convertType(node.body->t) << " 0" << std::endl;
        symbolTable.exitScope();
    }

    void visit(ast::Funcs &node) override {
        for (auto &func : node.funcs) {
            func->accept(*this);
        }
    }


    bool inLoop = false;
private:
    std::string breakLabel;
    std::string continueLabel;
};

#endif // CODEGEN_VISITOR_HPP