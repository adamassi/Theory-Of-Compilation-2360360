#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "nodes.hpp"
#include <iostream>

class SymbolTable {
public:
//constructor
    SymbolTable():  currentOffset(0) , functions() , scopes() {
        functions["print"] = {ast::BuiltInType::VOID, {ast::BuiltInType::INT}};
        functions["printi"] = {ast::BuiltInType::VOID, {ast::BuiltInType::INT}};
        
    }
    //add variable to the current scope be formals
    void addVariablef(const std::string &name, ast::BuiltInType type) {
        currentScope().variables[name] = {type, currentOffset--};
    }
    bool addVariable(const std::string &name, ast::BuiltInType type) {
        if (currentScope().variables.count(name) > 0) {
            return false;
        }
        // print number of scopes
        // std::cout << scopes.size() << std::endl;
        // //print all variables for dubug
        //  for (auto it = currentScope().variables.begin(); it != currentScope().variables.end(); ++it) {
        //     std::cout << it->first << std::endl;
        // }

        currentScope().variables[name] = {type, currentOffset++};
        return true;
    }
    void addFunctions(ast::FuncDecl &node){
        //for dubug addFunctions
        //std::cout << "addFunctions" << std::endl;
        //std::cout << node.id->value << std::endl;
        functions[node.id->value] = {node.return_type->type, {}};
        for (auto &formal : node.formals->formals) {
            functions[node.id->value].paramTypes.push_back(formal->type->type);
        }
        //print all functions for dubug 
        // for (auto it = functions.begin(); it != functions.end(); ++it) {
        //     std::cout << it->first << std::endl;
        // }
        //std::cout << "done addFunctions" << std::endl;
    }
    bool addFunction(const std::string &name, ast::BuiltInType returnType) {
       // std::cout << "addFunction" << std::endl;
        //std::cout << name << std::endl;
        // print all functions for dubug
        // for (auto it = functions.begin(); it != functions.end(); ++it) {
        //     std::cout << it->first << std::endl;
        // }
        //std::cout << "done" << std::endl;
        if (functions.count(name) > 0) {
            return false;
        }
        functions[name] = {returnType, {}};
        return true;
    }

    bool isVariableDefined(const std::string &name) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->variables.count(name) > 0) {
                return true;
            }
        }
        return false;
    }

    bool isFunctionDefined(const std::string &name) const {
        // Check if function is defined in the global buffer
        // print all functions
        for (auto it = functions.begin(); it != functions.end(); ++it) {
            //std::cout << it->first << std::endl;
            if(it->first == name){
                
                //std::cout <<name << "true"<<std::endl;
                return true;
            }
        }
        //std::cout << name << std::endl;
       // std::cout << "false"<<functions.count(name) << std::endl;
        
         
        return false;
    }

    int getOffset(const std::string &name) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto varIt = it->variables.find(name);
            if (varIt != it->variables.end()) {
                return varIt->second.offset;
            }
        }
        return -1;
    }

    std::vector<ast::BuiltInType> getParamTypes(const std::string &name) const {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second.paramTypes;
        }
        return {};
    }

    void enterScope() {
        currentOffset = 0;
        scopes.push_back(Scope());
    }
    //for new function scope
    void enterfScope() {
        currentOffset = -1;
        scopes.push_back(Scope());
    }
    // for loop scope
    void enterlScope() {
        scopes.push_back(Scope());
    }
    void exitScope() {
        scopes.pop_back();
    }


    struct Variable {
        ast::BuiltInType type;
        int offset;
    };

    struct Function {
        ast::BuiltInType returnType;
        std::vector<ast::BuiltInType> paramTypes;
    };

    struct Scope {
        std::unordered_map<std::string, Variable> variables;
    };

    std::vector<Scope> scopes;
    std::unordered_map<std::string, Function> functions;

    //symbolTable.addFunction(node.id->value, node.return_type->type)
    int currentOffset = 0;

    Scope &currentScope() {
        return scopes.back();
    }
};

#endif // SYMBOL_TABLE_HPP