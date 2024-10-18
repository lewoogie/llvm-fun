#include "CodeGen.h"        // Includes the code generation logic.
#include "Parser.h"         // Includes the parser logic to build the AST from the token stream.
#include "Sema.h"           // Includes the semantic analysis logic.
#include "llvm/Support/CommandLine.h"   // Provides command-line argument handling.
#include "llvm/Support/InitLLVM.h"      // Initializes the LLVM environment.
#include "llvm/Support/raw_ostream.h"   // Provides LLVM's output streams (like `llvm::errs()` for errors).

// Define a command-line option for the input expression.
static llvm::cl::opt<std::string>
    Input(llvm::cl::Positional,              // Positional argument (required input from the command line).
          llvm::cl::desc("<input expression>"),  // Description of the argument shown in the help message.
          llvm::cl::init(""));               // Default value if no input is provided.

// The main function drives the compilation process.
int main(int argc, const char **argv) {
  // Initialize the LLVM environment with command-line arguments.
  llvm::InitLLVM X(argc, argv);
  
  // Parse the command-line options. 
  // This processes the input expression provided via the command line.
  llvm::cl::ParseCommandLineOptions(
      argc, argv, "calc - the expression compiler\n");  // Displays the name and description for the tool.

  // Step 1: Lexical Analysis
  // The lexer takes the input expression and breaks it into tokens.
  Lexer Lex(Input);

  // Step 2: Parsing
  // The parser takes the tokens from the lexer and produces an Abstract Syntax Tree (AST).
  Parser Parser(Lex);
  AST *Tree = Parser.parse();  // Parse the input and get the AST.
  
  // Check if the parsing resulted in any errors or if the AST is null.
  if (!Tree || Parser.hasError()) {
    llvm::errs() << "Syntax errors occured\n";  // Report syntax errors if found.
    return 1;  // Exit with an error code.
  }

  // Step 3: Semantic Analysis
  // Perform semantic analysis to ensure correctness (e.g., all variables are declared).
  Sema Semantic;
  if (Semantic.semantic(Tree)) {
    llvm::errs() << "Semantic errors occured\n";  // Report semantic errors if found.
    return 1;  // Exit with an error code.
  }

  // Step 4: Code Generation
  // If the syntax and semantics are correct, generate the LLVM IR code.
  CodeGen CodeGenerator;
  CodeGenerator.compile(Tree);  // Compile the AST to LLVM IR and print the generated code.

  return 0;  // Return 0 to indicate success.
}

