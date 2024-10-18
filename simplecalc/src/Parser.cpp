#include "Parser.h"

// This is the top-level parse function.
// It parses the entire input, expecting the end of input (eoi) after the calculation.
// Returns the root node of the AST.
AST *Parser::parse() {
  AST *Res = parseCalc();  // Parse a calculation (expression or statement).
  expect(Token::eoi);      // Ensure that the token stream ends after parsing.
  return Res;              // Return the AST (or nullptr if parsing failed).
}

// Parses a "calculation" which can either be a simple expression or a "with" declaration.
AST *Parser::parseCalc() {
  Expr *E;
  llvm::SmallVector<llvm::StringRef, 8> Vars;  // A small vector to store variable names.
  
  // Check if the input starts with the "with" keyword for a variable declaration.
  if (Tok.is(Token::KW_with)) {
    advance();  // Consume the "with" keyword.
    
    // Expect an identifier (variable name).
    if (expect(Token::ident))
      goto _error;  // If there's an error, jump to the error-handling section.
    
    Vars.push_back(Tok.getText());  // Add the variable name to the Vars list.
    advance();  // Move to the next token.
    
    // Continue parsing more identifiers separated by commas (for multiple variables).
    while (Tok.is(Token::comma)) {
      advance();  // Consume the comma.
      if (expect(Token::ident))
        goto _error;  // Expect another identifier after the comma.
      Vars.push_back(Tok.getText());  // Add the variable name to the Vars list.
      advance();  // Move to the next token.
    }
    
    // Expect a colon after the variables list.
    if (consume(Token::colon))
      goto _error;  // If the colon is missing, go to error handling.
  }
  
  // Parse the expression after "with" (or if there's no "with").
  E = parseExpr();  // Parse the expression.
  
  // Ensure that the expression is followed by the end of input (eoi).
  if (expect(Token::eoi))
    goto _error;  // If not followed by eoi, handle the error.
  
  // If there were no "with" variables, return the parsed expression as the result.
  if (Vars.empty())
    return E;
  else
    // If there were "with" variables, return a WithDecl node with the variables and expression.
    return new WithDecl(Vars, E);
  
// Error handling block.
_error:
  // If an error occurs, consume tokens until the end of input (eoi) to clean up.
  while (Tok.getKind() != Token::eoi)
    advance();  // Keep advancing until eoi is reached.
  return nullptr;  // Return nullptr to indicate a failure in parsing.
}

// Parses an expression, which is a term followed by optional addition or subtraction operators.
Expr *Parser::parseExpr() {
  Expr *Left = parseTerm();  // Start by parsing the first term.
  
  // Continue parsing as long as the current token is a plus or minus operator.
  while (Tok.isOneOf(Token::plus, Token::minus)) {
    // Determine the type of binary operation (addition or subtraction).
    BinaryOp::Operator Op = Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
    advance();  // Consume the operator.
    
    // Parse the next term after the operator.
    Expr *Right = parseTerm();
    
    // Combine the left and right terms into a binary operation node (e.g., Left + Right).
    Left = new BinaryOp(Op, Left, Right);
  }
  
  return Left;  // Return the resulting expression (could be a single term or a binary operation).
}

// Parses a term, which is a factor followed by optional multiplication or division operators.
Expr *Parser::parseTerm() {
  Expr *Left = parseFactor();  // Start by parsing the first factor.
  
  // Continue parsing as long as the current token is a multiplication or division operator.
  while (Tok.isOneOf(Token::star, Token::slash)) {
    // Determine the type of binary operation (multiplication or division).
    BinaryOp::Operator Op = Tok.is(Token::star) ? BinaryOp::Mul : BinaryOp::Div;
    advance();  // Consume the operator.
    
    // Parse the next factor after the operator.
    Expr *Right = parseFactor();
    
    // Combine the left and right factors into a binary operation node (e.g., Left * Right).
    Left = new BinaryOp(Op, Left, Right);
  }
  
  return Left;  // Return the resulting term (could be a single factor or a binary operation).
}

// Parses a factor, which could be a number, identifier, or parenthesized expression.
Expr *Parser::parseFactor() {
  Expr *Res = nullptr;  // Initialize the result to null.
  
  // Switch on the kind of the current token to determine what type of factor it is.
  switch (Tok.getKind()) {
  
  // If the token is a number, create a Factor node for the number.
  case Token::number:
    Res = new Factor(Factor::Number, Tok.getText());  // Store the number's value.
    advance();  // Consume the number token.
    break;
  
  // If the token is an identifier, create a Factor node for the identifier.
  case Token::ident:
    Res = new Factor(Factor::Ident, Tok.getText());  // Store the identifier's name.
    advance();  // Consume the identifier token.
    break;
  
  // If the token is a left parenthesis, parse the expression inside the parentheses.
  case Token::l_paren:
    advance();  // Consume the left parenthesis.
    Res = parseExpr();  // Parse the expression inside the parentheses.
    
    // After parsing the expression, ensure that it is followed by a right parenthesis.
    if (!consume(Token::r_paren)) break;  // If the right parenthesis is missing, handle the error.
  
  // Default case to handle errors.
  default:
    if (!Res)
      error();  // If no valid factor was parsed, report an error.
    
    // Advance through the tokens until one that makes sense in the context (e.g., next operator or end).
    while (!Tok.isOneOf(Token::r_paren, Token::star, Token::plus, Token::minus, Token::slash, Token::eoi))
      advance();  // Skip tokens until reaching a valid end point (e.g., next operator or end of input).
  }
  
  return Res;  // Return the resulting factor (or null if there was an error).
}

