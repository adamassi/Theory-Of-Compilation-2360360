#include "output.hpp"
#include "nodes.hpp"
#include "semantic_visitor.hpp"
#include "decl.hpp"

// Extern from the bison-generated parser
extern int yyparse();
extern std::shared_ptr<ast::Node> program;

// Define the symbolTable instance
SymbolTable symbolTable;

int main() {
    // Parse the input. The result is stored in the global variable `program`
    yyparse();

    output::ScopePrinter scopePrinter;
    // Perform semantic analysis
    SemanticVisitor semanticVisitor(scopePrinter);
    // Add print and printi functions to global buffer
    scopePrinter.emitFunc("print", ast::BuiltInType::VOID, {ast::BuiltInType::STRING});
    scopePrinter.emitFunc("printi", ast::BuiltInType::VOID, {ast::BuiltInType::INT});
    program->accept(semanticVisitor);

    

    std::cout << scopePrinter;

    return 0;
}
