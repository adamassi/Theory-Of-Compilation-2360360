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
    std::string baseReg; // Base register for stack
    std::string current_func; // Label for break statements

    CodeGenVisitor(output::CodeBuffer &buffer, SymbolTable &symbolTable)
        : buffer(buffer), symbolTable(symbolTable) {
        
    }

    // Load variables from stack
    //in is the type of the variable
    std::string load_from_stack(std::string reg,int offset, std::string in) {

        
        std::string tmp = buffer.freshVar();
        string tmp_reg = reg;
        if (offset < 0) {
            tmp_reg= "%" + to_string(-1 - offset);
             std::string res = buffer.freshVar();
             if (in == "i1") {
                /* code */
                std::string tmp = buffer.freshVar();
                buffer << tmp << " = zext i1 " << tmp_reg << " to i32" << std::endl;
                tmp_reg = tmp;
                

             }
             if (in == "i8"){
                /* code */
                std::string tmp = buffer.freshVar();
                buffer << tmp << " = zext i8 " << tmp_reg << " to i32" << std::endl;
                tmp_reg = tmp;
                
             }
             
             
             buffer << res << " = add " <<"i32" << " 0, " << tmp_reg << std::endl;
        
            return res;
    }
        buffer << tmp << " = getelementptr i32, i32* " << baseReg << ", i32 " << offset << std::endl;
        std::string var = buffer.freshVar();
        buffer << var << " = load i32, i32* " << tmp << std::endl;
        return var;
    }

    // Store variables to stack
    void store_to_stack(std::string value,int offset, BuiltInType type,BuiltInType type_right=BuiltInType::VOID) {
        
        std::string var = value;
        if (type == BuiltInType::BYTE) {
            var = buffer.freshVar();
            buffer << var << " = zext i8 " << value << " to i32" << std::endl; 
        }
        if (type == BuiltInType::BOOL)
        {
            var = buffer.freshVar();
            buffer << var << " = zext i1 " << value << " to i32" << std::endl; 
        }
        if (type == BuiltInType::INT && type_right == BuiltInType::BYTE) {
            var = buffer.freshVar();
            buffer << var << " = zext i8 " << value << " to i32" << std::endl; 
        }
        
        std::string tmp = buffer.freshVar();
        buffer << tmp << " = getelementptr i32, i32* " << baseReg << ", i32 " << offset << std::endl;
        buffer << "store i32 " << var << ", i32* " << tmp << std::endl;
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
         //cout << "NumB" <<node.type << endl;
        node.type = BuiltInType::BYTE;
        if (node.value > 255) {
            output::errorByteTooLarge(node.line, node.value);
        }
        std::string var = buffer.freshVar();
        buffer << var << " = add i8 0, " << node.value << std::endl;
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
        if (!symbolTable.isVariableDefined(node.value)) {
            output::errorUndef(node.line, node.value);
        } else {
            node.type = symbolTable.getVariableType(node.value);
        }
        std::string var;
        std::string type = convertType(node.type);
        int offset = symbolTable.getOffset(node.value);

        var = load_from_stack(baseReg, offset, type);

        if (node.type == BuiltInType::BYTE) {
            std::string var2 = buffer.freshVar();
            buffer << var2 << " = trunc i32 " << var << " to i8" << std::endl;
            var = var2;
        }
        if (node.type == BuiltInType::BOOL) {
            std::string var2 = buffer.freshVar();
            buffer << var2 << " = trunc i32 " << var << " to i1" << std::endl;
            var = var2;
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
        std::string varLeft = node.left->code;
        
        if(node.left->type == BuiltInType::BYTE){
            varLeft = buffer.freshVar();
            buffer << varLeft << " = zext i8 " + node.left->code + " to i32" << std::endl;

        }
        std::string varRight = node.right->code;
        if(node.right->type == BuiltInType::BYTE){
            varRight = buffer.freshVar();
            buffer << varRight << " = zext i8 " + node.right->code + " to i32" << std::endl;
            
        }

        switch (node.op) {
            case ast::BinOpType::ADD:
                    buffer << var << " = add i32 " << varLeft << ", " << varRight << std::endl;
                break;
            case ast::BinOpType::SUB:
                buffer << var << " = sub i32 " << varLeft << ", " << varRight << std::endl;
                break;
            case ast::BinOpType::MUL:
                buffer << var << " = mul i32 " << varLeft << ", " << varRight << std::endl;
                break;
                
                
            case ast::BinOpType::DIV:
            // if (node.left->type == BuiltInType::INT) {
            //     buffer << var << " = sdiv i32 " << node.left->code << ", " << node.right->code << std::endl;
            // } else {
            //     buffer << var << " = udiv i32 " << node.left->code << ", " << node.right->code << std::endl;
            // }
                std::string extendedRight = buffer.freshVar();
                buffer << extendedRight << " = icmp ne i32 " << varRight << ", 0" << std::endl;
                //node.right->code = extendedRight;
                //buffer.emitLabel("%div_err");
                //buffer << "call void @divide_by_zero_error()" << std::endl;
                // buffer << "br label %div_ok" << std::endl;
                // buffer.emitLabel("%div_ok");
                std::string lacelDiv = buffer.freshLabel();
                buffer << "br i1 " << extendedRight << ", label "<<lacelDiv<<", label "<<"%"<<"div_err"<<current_func << std::endl;
                buffer.emitLabel(lacelDiv);
                buffer << var << " = sdiv i32 " << varLeft << ", " << varRight << std::endl;
                
        
                break;
                
        }
        if(node.right->type == BuiltInType::BYTE && node.left->type == BuiltInType::BYTE){
            std::string var2 = buffer.freshVar();
            buffer << var2 << " = trunc i32 " << var << " to i8" << std::endl;
            var = var2;
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
        std::string varLeft = node.left->code;
        node.type = ast::BuiltInType::BOOL;
        if(node.left->type == BuiltInType::BYTE){
            varLeft = buffer.freshVar();
            buffer << varLeft << " = zext i8 " + node.left->code + " to i32"<< std::endl;

        }
        std::string varRight = node.right->code;
        if(node.right->type == BuiltInType::BYTE){
            varRight = buffer.freshVar();
            buffer << varRight << " = zext i8 " + node.right->code + " to i32" << std::endl;
            
        }
        std::string var = buffer.freshVar();
        switch (node.op) {
            //EQ: equal
            case ast::RelOpType::EQ:
                buffer << var << " = icmp eq i32 " << varLeft << ", " << varRight << std::endl; //eq
                break;
            //NE:not equal
            case ast::RelOpType::NE:
                buffer << var << " = icmp ne i32 " << varLeft << ", " << varRight << std::endl; //ne 
                break;
            //LT: less than <
            case ast::RelOpType::LT:
                buffer << var << " = icmp slt i32 " << varLeft << ", " << varRight << std::endl; //sge//slt
                break;
            //GT:greater than >
            case ast::RelOpType::GT:
                buffer << var << " = icmp sgt i32 " << varLeft << ", " << varRight << std::endl; //sle
                break;
            //LE:<=
            case ast::RelOpType::LE:
                buffer << var << " = icmp sle i32 " << varLeft << ", " << varRight << std::endl; //sgt
                break;
            //GE:>=
            case ast::RelOpType::GE:
                buffer << var << " = icmp sge i32 " << varLeft << ", " << varRight << std::endl; //slt 
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
        std::string rightCond = buffer.freshLabel();//0
        std::string labelTrue = buffer.freshLabel();//1
        std::string labelFalse = buffer.freshLabel();//2
        std::string labelEnd = buffer.freshLabel();//3
        std::string var = buffer.freshVar();
        std::string old = node.t_label;
        node.t_label = labelTrue;
        node.f_label = labelEnd;
        buffer << "br i1 " << node.left->code << ", label " << rightCond << ", label " << labelFalse << std::endl;
        buffer.emitLabel(rightCond);
        node.right->accept(*this);
        if (node.right->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        node.type = BuiltInType::BOOL;
        buffer << "br i1 " << node.right->code << ", label " << labelTrue << ", label " << labelFalse << std::endl;
        //buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelTrue);
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelFalse);
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelEnd);
        buffer << var << " = phi i1 [ 1, " << labelTrue << " ], [ 0, " << labelFalse << " ]" << std::endl;
        node.code = var;
    }

    void visit(ast::Or &node) override {
        node.left->accept(*this);
        if (node.left->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        //std::string labelTrue1 = buffer.freshLabel();//0
        //std::string labelFalse1 = buffer.freshLabel();//1
        //std::string labelTrue2 = buffer.freshLabel();//2
        //std::string labelFalse2 = buffer.freshLabel();//3
        std::string rightCond = buffer.freshLabel();//0
        std::string labelTrue = buffer.freshLabel();//4//2
        std::string labelFalse = buffer.freshLabel();//5//2
        std::string labelEnd = buffer.freshLabel();//6//3
        std::string var = buffer.freshVar();
        node.t_label = labelTrue;
        node.f_label = labelEnd;
        //chech if the left side is true or false and go to the right side if it is false
        buffer << "br i1 " << node.left->code << ", label " << labelTrue << ", label " << rightCond << std::endl;
        buffer.emitLabel(rightCond);
        node.right->accept(*this);

        // buffer.emitLabel(labelTrue);//0
        // buffer << "br label " << labelTrue << std::endl;
        // buffer.emitLabel(labelFalse1);//1
        //node.right->accept(*this);
        if (node.right->type != BuiltInType::BOOL) {
            output::errorMismatch(node.line);
        }
        node.type = BuiltInType::BOOL;
        buffer << "br i1 " << node.right->code << ", label " << labelTrue << ", label " << labelFalse << std::endl;
        // buffer.emitLabel(labelTrue2);//2
        // buffer << "br label " << labelTrue << std::endl;
        // buffer.emitLabel(labelFalse2);//3
        // buffer << "br label " << labelFalse << std::endl;
        buffer.emitLabel(labelTrue);//4
        //std::string trueVar = buffer.freshVar();
        //buffer << trueVar << " = add i1 0, 1" << std::endl;
        buffer << "br label " << labelEnd << std::endl;
        buffer.emitLabel(labelFalse);//5
        //std::string falseVar = buffer.freshVar();
        //buffer << falseVar << " = add i1 0, 0" << std::endl;
        buffer << "br label " << labelEnd << std::endl;
        
        buffer.emitLabel(labelEnd);
        buffer << var << " = phi i1 [1, "  << " " << labelTrue << " ], [ 0, " << labelFalse << " ]" << std::endl;
        // if(node.right->f_label == "") {
        //     buffer << var << " = phi i1 [ 1, " << labelTrue << " ], [ " << node.right->code << ", " << labelFalse << " ]" << std::endl;
        // } 
        // else {
        //     buffer << var << " = phi i1 [ 1, " << labelTrue << " ], [ " << node.right->code << ", " << node.right->f_label << " ]" << std::endl;
        // }
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
                        // for last test 
                        // if(num->value > 255){
                        //     output::errorByteTooLarge(node.line, num->value);
                        // }
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
        std::vector<ast::BuiltInType> args_types = symbolTable.getParamTypes(node.func_id->value);
        int i =0;
        for (auto &arg : node.args->exps) {
            
            // go to the function arguments and find if the argument is type or not.
            // if the function argument in the decleration is byte and the current argument is also byte, dont do anything
            // if the function argument in the declaration is int and the current argumen type is byte then add zext command 
            // 
            //  functions[node.id->value].paramTypes
            if(args_types[i] == BuiltInType::INT && arg->type == BuiltInType::BYTE){
                std::string var2 = buffer.freshVar();
                buffer << var2 << " = zext i8 " << arg->code << " to i32" << std::endl;
                arg->code = var2;
            }
                
            args += convertType(args_types[i]) +" "+arg->code + ", ";
            i++;

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

        // if (node.type == BuiltInType::BYTE) {
        // std::string var_ext = buffer.freshVar();
        // buffer << var_ext << " = zext i8 " << var << " to i32" << std::endl;
        // var = var_ext;
        // }

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
                    //cout << "error mismatch return" << endl;
                    //cout << "currentType" << currentType << endl;
                    output::errorMismatch(node.line);
                }
                else
                {
                    std::string var = buffer.freshVar();
                    buffer << var << " = zext i8 " << node.exp->code << " to i32" << std::endl;
                    node.exp->code = var;
                }
            }
            buffer << "ret " << convertType(currentType) << " " << node.exp->code << std::endl;
        } else {
            
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
            //buffer << "br label " << labelEnd << std::endl;
        }
         buffer << "br label " << labelEnd << std::endl;
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
        //cout << "VarDecl" << endl;
         node.type->accept(*this);
        // node.init_exp if the right side of the assignment is not null
        if (node.init_exp) {
            //cout << "node.init_exp" << endl;
            //han bnro7 3la numb
            if (dynamic_cast<ast::ID*>(node.init_exp.get())) {
                if (symbolTable.isFunctionDefined(dynamic_cast<ast::ID*>(node.init_exp.get())->value)) {
                    //std::cout << "errorDefAsFunc" << std::endl;
                    output::errorDefAsFunc(node.line, dynamic_cast<ast::ID*>(node.init_exp.get())->value);
                }
                if(!symbolTable.isVariableDefined(dynamic_cast<ast::ID*>(node.init_exp.get())->value)){
                    //cout << "error LINE 621" << endl;
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
           
             int offset = symbolTable.addVariableByOffset(node.id->value, node.type->type);
            //cout << "offset::" << offset << endl;
            if (offset<0){
                //cout << "error 647" << endl;
                output::errorDef(node.line, node.id->value);
            }
            

            //cout << "offset::" << offset << endl;
            store_to_stack(node.init_exp->code, offset, node.type->type,node.init_exp->type );
        

        // if (!symbolTable.addVariable(node.id->value, node.type->type)) {
        //     output::errorDef(node.line, node.id->value);
        // }
        
        //symbolTable.addVariable(node.id->value, node.type->type);
        }
        else {
            int offset = symbolTable.addVariableByOffset(node.id->value, node.type->type);
            if (offset<0){
                output::errorDef(node.line, node.id->value);
            }
            store_to_stack("0", offset, node.type->type);
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
        int offset = symbolTable.getOffset(node.id->value);
        store_to_stack(node.exp->code, offset, node.id->type,node.exp->type);
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
                buffer << convertTypeString(paramTypes[i]) ;
            }
            else{
                buffer << ", " << convertTypeString(paramTypes[i]) ;
            }
            
            i++;
        }
        i=0;
        // if (!node.formals->formals.empty()) {
        //     buffer.seekp(-2, std::ios_base::end);
        // }
        buffer << ") {" << std::endl;
        // create stack base reg
        //std::string frsh = buffer.freshVar().substr(1);
        baseReg = "stackReg_" + node.id->value;
        baseReg = "%" + baseReg;
        //if (node.id->value == "main") {
           
        
        
            /* code */
        
        current_func=node.id->value;
        buffer << "br label %div_ok"<<node.id->value << std::endl;
        buffer.emitLabel("%div_err"+node.id->value);
        buffer << "call void @divide_by_zero_error()" << std::endl;
        buffer << "br label %div_ok"<<node.id->value << std::endl;
        buffer.emitLabel("%div_ok"+node.id->value);
        //}
        buffer << baseReg << " = alloca i32, i32 50" << std::endl;
        node.baseReg = baseReg;
        symbolTable.enterScope();
        node.formals->accept(*this);
        node.body->t = node.return_type->type;
        for (auto &formal : node.formals->formals) {
            //std::string var = "%" + formal->id->value;
            std::string type = convertType(formal->type->type);
            int offset = symbolTable.getOffset(formal->id->value);
            std::string var = "%" + to_string(-1 - offset);
            //store_to_stack(var, offset, formal->type->type);
        }
        
        //cout << "body" << endl;
        node.body->accept(*this);
        //if (!hasReturn) {

            if (node.return_type->type == BuiltInType::VOID) {
                buffer << "ret void" << std::endl;
            } 
            if(node.return_type->type == BuiltInType::INT){
                buffer << "ret i32 0" << std::endl;
            }
            if(node.return_type->type == BuiltInType::BYTE){
                buffer << "ret i8 0" << std::endl;
            }
            if(node.return_type->type == BuiltInType::BOOL){
                buffer << "ret i1 0" << std::endl;
            }
       // }
        

        buffer << "}" << std::endl;
        if (node.return_type->type != node.body->t) {
           //cout << "error mismatch" << endl;
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