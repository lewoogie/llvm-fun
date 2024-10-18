#include "Sema.h"
#include "llvm/ADT/StringSet.h"        // LLVM's StringSet is used to track variable names in the scope.
#include "llvm/Support/raw_ostream.h"  // For error reporting using llvm's output streams.

namespace {

// DeclCheck class performs semantic analysis by visiting AST nodes to check variable declarations and usages.
class DeclCheck : public ASTVisitor {
  llvm::StringSet<> Scope;  // A set of declared variables to track which variables are in scope.
  bool HasError;            // Flag indicating whether an error has been encountered.

  // Enumeration to represent different types of semantic errors.
  enum ErrorType { Twice, Not };

  // Function to report errors about variable declarations or usages.
  void error(ErrorType ET, llvm::StringRef V) {
    // Print an error message to the error stream indicating the variable and the type of error.
    llvm::errs() << "Variable " << V << " "
                 << (ET == Twice ? "already" : "not")
                 << " declared\n";
    HasError = true;  // Set the HasError flag to true.
  }

public:
  // Constructor to initialize the HasError flag.
  DeclCheck() : HasError(false) {}

  // Returns whether any semantic errors have been found during the analysis.
  bool hasError() { return HasError; }

  // Visit a Factor node (which represents an identifier or literal).
  // This checks if an identifier (variable) has been declared.
  virtual void visit(Factor &Node) override {
    // Check if the Factor node is an identifier.
    if (Node.getKind() == Factor::Ident) {
      // If the identifier is not found in the scope, it hasn't been declared.
      if (Scope.find(Node.getVal()) == Scope.end())
        error(Not, Node.getVal());  // Report an error for an undeclared variable.
    }
  };

  // Visit a BinaryOp node (which represents a binary operation like +, -, *, /).
  // This ensures that both the left and right operands of the binary operation are valid expressions.
  virtual void visit(BinaryOp &Node) override {
    // Visit the left operand of the binary operation.
    if (Node.getLeft())
      Node.getLeft()->accept(*this);  // Recursively check the left expression.
    else
      HasError = true;  // If the left operand is missing, set the HasError flag.

    // Visit the right operand of the binary operation.
    if (Node.getRight())
      Node.getRight()->accept(*this);  // Recursively check the right expression.
    else
      HasError = true;  // If the right operand is missing, set the HasError flag.
  };

  // Visit a WithDecl node (which represents a "with" declaration like `with x, y: <expr>`).
  // This checks if the declared variables are unique and adds them to the scope.
  virtual void visit(WithDecl &Node) override {
    // Iterate over the variables declared in the "with" statement.
    for (auto I = Node.begin(), E = Node.end(); I != E; ++I) {
      // Try to insert the variable into the scope. If the insertion fails, the variable is already declared.
      if (!Scope.insert(*I).second)
        error(Twice, *I);  // Report an error for redeclaring a variable.
    }

    // Visit the expression that follows the "with" declaration.
    if (Node.getExpr())
      Node.getExpr()->accept(*this);  // Recursively check the expression.
    else
      HasError = true;  // If there's no expression, set the HasError flag.
  };
};

}

// The Sema (semantic analysis) function performs the semantic analysis on the given AST.
// It returns true if any semantic errors were found, false otherwise.
bool Sema::semantic(AST *Tree) {
  // If the AST is null, return false (no semantic analysis can be performed).
  if (!Tree)
    return false;

  // Create an instance of DeclCheck to perform the semantic analysis.
  DeclCheck Check;

  // Start the semantic analysis by visiting the root of the AST.
  Tree->accept(Check);

  // Return whether any errors were found during the analysis.
  return Check.hasError();
}

