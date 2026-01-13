#include "output.hpp"
// #include "kosAmhom.hpp"
#include "nodes.hpp"
//#include "semantic_visitor.hpp"
#include "codegen_visitor.hpp"
#include "decl.hpp"

// Extern from the bison-generated parser
extern int yyparse();
extern std::shared_ptr<ast::Node> program;
output::CodeBuffer codeBuffer;

// Define the symbolTable instance
SymbolTable symbolTable;

void init_buffer(){
//    // output::CodeBuffer codeBuffer;
//   string errorMsg = "Error division by zero";

//   string declarations = "declare i32 @printf(i8*, ...)\n";
//   declarations += "declare void @exit(i32)\n";
//   declarations += "@div_by_zero_error = constant [" + std::to_string(errorMsg.length()+1) + " x i8] c\"" + errorMsg + "\\00\"";
//   codeBuffer.emitString(declarations);

//   string specifiers = "@.str_specifier = internal constant [4 x i8] c\"%s\\0A\\00\"\n";
//   specifiers += "@.int_specifier = internal constant [4 x i8] c\"%d\\0A\\00\"\n";
//   codeBuffer.emitString(specifiers);

//   string print = "define void @print(i8*) {\n";
//   print += "    call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)\n";
//   print += "    ret void\n}";
//   codeBuffer.emit(print);

//   string printi = "define void @printi(i32) {\n";
//   printi += "    call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)\n";
//   printi += "    ret void\n}";
//   codeBuffer.emit(printi);
    codeBuffer.emit("declare i32 @scanf(i8*, ...)");
    codeBuffer.emit("declare i32 @printf(i8*, ...)");
    codeBuffer.emit("declare void @exit(i32)");
    codeBuffer.emit("@.int_specifier_scan = constant [3 x i8] c\"%d\\00\"");
    codeBuffer.emit("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    codeBuffer.emit("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");

    codeBuffer.emit("define i32 @readi(i32) {");
    codeBuffer.emit("    %ret_val = alloca i32");
    codeBuffer.emit("    %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0");
    codeBuffer.emit("    call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)");
    codeBuffer.emit("    %val = load i32, i32* %ret_val");
    codeBuffer.emit("    ret i32 %val");
    codeBuffer.emit("}");

    codeBuffer.emit("define void @printi(i32) {");
    codeBuffer.emit("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    codeBuffer.emit("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    codeBuffer.emit("    ret void");
    codeBuffer.emit("}");

    codeBuffer.emit("define void @print(i8*) {");
    codeBuffer.emit("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    codeBuffer.emit("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    codeBuffer.emit("    ret void");
    codeBuffer.emit("}");

    // Add the divide_by_zero_error function declaration at the top
        codeBuffer.emit("@divide_by_zero_message = constant [23 x i8] c\"Error division by zero\\00\"" );
        codeBuffer.emit("define void @divide_by_zero_error() {");
        codeBuffer.emit("    %divide_by_zero_ptr = getelementptr [23 x i8], [23 x i8]* @divide_by_zero_message, i32 0, i32 0");
        codeBuffer.emit("    call void @print(i8* %divide_by_zero_ptr)");
        codeBuffer.emit("    call void @exit(i32 0)");
        codeBuffer.emit("    ret void");
        codeBuffer.emit("}");
  
}   

int main() {
    init_buffer();
    // Parse the input. The result is stored in the global variable `program`
    yyparse();
    // kosAmhom::ScopePrinter scopePrinter;
    // output::CodeBuffer codeBuffer;
    // Perform semantic analysis
    // SemanticVisitor semanticVisitor(scopePrinter);
    //program->accept(semanticVisitor);

    // Generate LLVM IR code
    CodeGenVisitor codeGenVisitor(codeBuffer, symbolTable);
    //codeBuffer.emitString("print", ast::BuiltInType::VOID, {ast::BuiltInType::STRING});
    //codeBuffer.emit("printi", ast::BuiltInType::VOID, {ast::BuiltInType::INT});

    

    program->accept(codeGenVisitor);


    if (!symbolTable.isFunctionDefined("main") || symbolTable.getFunctionType("main") != ast::BuiltInType::VOID||symbolTable.getParamTypes("main").size() != 0) {
        output::errorMainMissing();
    }


    // Print the generated code
    std::cout << codeBuffer;

    return 0;
}
