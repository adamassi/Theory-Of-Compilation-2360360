#include "output.hpp"
#include "nodes.hpp"
#include "semantic_visitor.hpp"

// Extern from the bison-generated parser
extern int yyparse();

extern std::shared_ptr<ast::Node> program;

int main() {
    // Parse the input. The result is stored in the global variable `program`
    yyparse();

    output::ScopePrinter scopePrinter;
    // Perform semantic analysis
    SemanticVisitor semanticVisitor(scopePrinter);
    
    program->accept(semanticVisitor);

    // add print and printi functions to global buffer
   
    scopePrinter.emitFunc("print", ast::BuiltInType::VOID, {ast::BuiltInType::INT});
    scopePrinter.emitFunc("printi", ast::BuiltInType::VOID, {ast::BuiltInType::INT});

     std::cout << scopePrinter;

    // Print the AST using the PrintVisitor
    //output::PrintVisitor printVisitor;
    //program->accept(printVisitor);
}
