#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"       // StringMap for tracking variable bindings (name to LLVM value).
#include "llvm/IR/IRBuilder.h"        // IRBuilder is used to generate LLVM instructions.
#include "llvm/IR/LLVMContext.h"      // LLVMContext provides a context for the generated IR.
#include "llvm/Support/raw_ostream.h" // For printing IR and debugging.

using namespace llvm;  // Import the LLVM namespace for easier access to LLVM types and functions.

namespace {

// ToIRVisitor class is responsible for visiting AST nodes and generating corresponding LLVM IR.
class ToIRVisitor : public ASTVisitor {
  Module *M;               // Pointer to the LLVM Module, which holds the IR code.
  IRBuilder<> Builder;      // IRBuilder is used to generate instructions in the current context.
  Type *VoidTy;             // LLVM type representing 'void'.
  Type *Int32Ty;            // LLVM type representing 32-bit integers.
  PointerType *PtrTy;       // LLVM type representing a generic pointer.
  Constant *Int32Zero;      // Constant representing the integer value 0 in 32-bit form.

  Value *V;                 // Current LLVM Value being generated (result of evaluating expressions).
  StringMap<Value *> nameMap;  // Map from variable names to LLVM values (used for tracking variables).

public:
  // Constructor for the ToIRVisitor, initializes types and constants.
  ToIRVisitor(Module *M) : M(M), Builder(M->getContext()) {
    // Initialize common LLVM types and constants.
    VoidTy = Type::getVoidTy(M->getContext());      // Void type.
    Int32Ty = Type::getInt32Ty(M->getContext());    // 32-bit integer type.
    PtrTy = PointerType::getUnqual(M->getContext()); // Generic pointer type.
    Int32Zero = ConstantInt::get(Int32Ty, 0, true); // Constant 0 for return in main.
  }

  // Runs the code generation process for the entire AST.
  void run(AST *Tree) {
    // Define the 'main' function with signature: int main(int, char**)
    FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, PtrTy}, false);
    Function *MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);
    BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
    Builder.SetInsertPoint(BB);  // Set the insertion point for IR generation in the entry block.

    // Visit the root of the AST to generate code for it.
    Tree->accept(*this);

    // Declare an external function 'calc_write(int)' and generate a call to it with the result 'V'.
    FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);
    Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "calc_write", M);
    Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {V});

    // Return 0 from the 'main' function.
    Builder.CreateRet(Int32Zero);
  }

  // Visit a Factor node (which could be an identifier or a literal).
  virtual void visit(Factor &Node) override {
    if (Node.getKind() == Factor::Ident) {
      // If the Factor is an identifier, look it up in the nameMap and assign the corresponding value to 'V'.
      V = nameMap[Node.getVal()];
    } else {
      // If the Factor is a number, convert the string to an integer and create a constant LLVM value.
      int intval;
      Node.getVal().getAsInteger(10, intval);  // Convert string to an integer.
      V = ConstantInt::get(Int32Ty, intval, true);  // Create a constant integer in LLVM IR.
    }
  }

  // Visit a BinaryOp node (which represents binary operations like +, -, *, /).
  virtual void visit(BinaryOp &Node) override {
    // First, visit the left operand and store its result in 'Left'.
    Node.getLeft()->accept(*this);
    Value *Left = V;  // Save the result of the left operand.

    // Then, visit the right operand and store its result in 'Right'.
    Node.getRight()->accept(*this);
    Value *Right = V;  // Save the result of the right operand.

    // Generate the appropriate LLVM instruction based on the binary operator.
    switch (Node.getOperator()) {
    case BinaryOp::Plus:
      V = Builder.CreateNSWAdd(Left, Right);  // Create a no-signed-wrap addition.
      break;
    case BinaryOp::Minus:
      V = Builder.CreateNSWSub(Left, Right);  // Create a no-signed-wrap subtraction.
      break;
    case BinaryOp::Mul:
      V = Builder.CreateNSWMul(Left, Right);  // Create a no-signed-wrap multiplication.
      break;
    case BinaryOp::Div:
      V = Builder.CreateSDiv(Left, Right);    // Create a signed division.
      break;
    }
  }

  // Visit a WithDecl node (which represents a "with" declaration).
  virtual void visit(WithDecl &Node) override {
    // Declare an external function 'calc_read(char*)' to read variable values from input.
    FunctionType *ReadFty = FunctionType::get(Int32Ty, {PtrTy}, false);
    Function *ReadFn = Function::Create(ReadFty, GlobalValue::ExternalLinkage, "calc_read", M);

    // For each variable in the "with" declaration, read its value.
    for (auto I = Node.begin(), E = Node.end(); I != E; ++I) {
      StringRef Var = *I;  // Get the variable name.

      // Create a global string constant for the variable name.
      Constant *StrText = ConstantDataArray::getString(M->getContext(), Var);
      GlobalVariable *Str = new GlobalVariable(*M, StrText->getType(), /*isConstant=*/true, GlobalValue::PrivateLinkage, StrText, Twine(Var).concat(".str"));

      // Generate a call to 'calc_read' to read the variable's value and store it in 'nameMap'.
      CallInst *Call = Builder.CreateCall(ReadFty, ReadFn, {Str});
      nameMap[Var] = Call;  // Store the result of the read in the nameMap for future lookups.
    }

    // Finally, visit the expression associated with the "with" declaration.
    Node.getExpr()->accept(*this);
  }
};
} // namespace

// The 'compile' function drives the code generation process.
// It creates an LLVM context and module, runs the ToIRVisitor on the AST, and prints the generated IR.
void CodeGen::compile(AST *Tree) {
  LLVMContext Ctx;                     // Create an LLVM context.
  Module *M = new Module("calc.expr", Ctx);  // Create a new LLVM module to hold the generated code.
  ToIRVisitor ToIR(M);                 // Create a ToIRVisitor instance to generate the IR.
  ToIR.run(Tree);                      // Run the code generation process on the given AST.
  M->print(outs(), nullptr);           // Print the generated LLVM IR to the output stream.
}

