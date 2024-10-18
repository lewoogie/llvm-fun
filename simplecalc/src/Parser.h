#ifndef PARSER_H
#define PARSER_H

#include "AST.h"         // Include the AST header, which defines the structures for abstract syntax tree nodes.
#include "Lexer.h"       // Include the Lexer header for tokenizing input.
#include "llvm/Support/raw_ostream.h"  // LLVM's output support for printing error messages or debugging.

class Parser {
  Lexer &Lex;        // Reference to the Lexer object. The parser interacts with the lexer to retrieve tokens.
  Token Tok;         // The current token that the parser is examining.
  bool HasError;     // A flag to indicate whether a parsing error has occurred.

  // Function to report an error when an unexpected token is encountered.
  void error() {
    // Output the error message along with the unexpected token text.
    llvm::errs() << "Unexpected: " << Tok.getText() << "\n";
    HasError = true;  // Set the error flag to true.
  }

  // Advance the lexer to the next token by calling `Lex.next`.
  void advance() { Lex.next(Tok); }

  // Expect a specific token kind. If the current token does not match, report an error.
  // Returns true if there was an error (i.e., the token did not match the expected kind).
  bool expect(Token::TokenKind Kind) {
    if (!Tok.is(Kind)) {  // Check if the current token matches the expected kind.
      error();            // Report an error if the token does not match.
      return true;        // Return true to indicate an error.
    }
    return false;         // No error, return false.
  }

  // Consume a token if it matches the expected kind. If it matches, the token is consumed, and the lexer advances to the next one.
  // Returns true if there was an error (i.e., the token did not match the expected kind).
  bool consume(Token::TokenKind Kind) {
    if (expect(Kind))     // Check if the token matches the expected kind.
      return true;        // If there's an error, return true immediately.
    advance();            // If the token matches, consume it by advancing the lexer.
    return false;         // No error, return false.
  }

  // Top-level function for parsing a complete calculation (e.g., an expression or statement).
  AST *parseCalc();

  // Parse an expression (could involve operators like '+' or '-').
  Expr *parseExpr();

  // Parse a term (could involve multiplication or division operators).
  Expr *parseTerm();

  // Parse a factor (basic building blocks like numbers, parentheses, etc.).
  Expr *parseFactor();

public:
  // Constructor that initializes the parser with a reference to a lexer.
  // It immediately advances the lexer to retrieve the first token.
  Parser(Lexer &Lex) : Lex(Lex), HasError(false) {
    advance();  // Start the lexer and get the first token.
  }

  // Parse the entire input, returning the root node of the abstract syntax tree (AST).
  AST *parse();

  // Check if there was any parsing error.
  bool hasError() { return HasError; }
};

#endif  // End of header guard

