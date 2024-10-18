#include "Lexer.h"

// Namespace containing utility functions for character classification.
namespace charinfo {

  // Returns true if the character is considered whitespace.
  // Whitespace characters: ' ', '\t', '\f', '\v', '\r', '\n'.
  LLVM_READNONE inline bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
           c == '\r' || c == '\n';
  }

  // Returns true if the character is a digit ('0' to '9').
  LLVM_READNONE inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
  }

  // Returns true if the character is a letter ('a' to 'z' or 'A' to 'Z').
  LLVM_READNONE inline bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }
}

// Function that advances the lexer to the next token in the input.
void Lexer::next(Token &token) {
  // Skip over any whitespace characters at the current buffer position.
  while (*BufferPtr && charinfo::isWhitespace(*BufferPtr)) {
    ++BufferPtr;  // Move to the next character in the buffer.
  }

  // If we reach the end of the buffer, set the token kind to `eoi` (end of input).
  if (!*BufferPtr) {
    token.Kind = Token::eoi;
    return;
  }

  // Check if the next token is an identifier (which starts with a letter).
  if (charinfo::isLetter(*BufferPtr)) {
    // Start scanning for a sequence of letters.
    const char *end = BufferPtr + 1;  // Move one position after the current.
    while (charinfo::isLetter(*end))  // Keep advancing while characters are letters.
      ++end;
    
    // Create a string from the current token's text (from BufferPtr to end).
    llvm::StringRef Name(BufferPtr, end - BufferPtr);

    // Check if the identifier is the keyword "with", otherwise it's a generic identifier.
    Token::TokenKind kind = (Name == "with") ? Token::KW_with : Token::ident;

    // Form the token with the identified kind.
    formToken(token, end, kind);
    return;

  // Check if the next token is a number (starts with a digit).
  } else if (charinfo::isDigit(*BufferPtr)) {
    // Start scanning for a sequence of digits.
    const char *end = BufferPtr + 1;
    while (charinfo::isDigit(*end))  // Keep advancing while characters are digits.
      ++end;

    // Form the token as a `number` token.
    formToken(token, end, Token::number);
    return;

  // If the next character isn't a letter or digit, check if it's a specific symbol.
  } else {
    switch (*BufferPtr) {
      // Handle single-character tokens using a macro to avoid repetition.
      // For each symbol, we create the corresponding token type.
#define CASE(ch, tok) \
case ch: formToken(token, BufferPtr + 1, tok); break

    // These cases handle characters like '+', '-', '*', '/', '(', ')', etc.
    CASE('+', Token::plus);
    CASE('-', Token::minus);
    CASE('*', Token::star);
    CASE('/', Token::slash);
    CASE('(', Token::Token::l_paren);
    CASE(')', Token::Token::r_paren);
    CASE(':', Token::Token::colon);
    CASE(',', Token::Token::comma);

#undef CASE  // End the macro definition.

    // If the character doesn't match any known token, mark it as `unknown`.
    default:
      formToken(token, BufferPtr + 1, Token::unknown);
    }
    return;
  }
}

// Helper function to create a token and advance the buffer pointer.
// `Tok`: The token to fill in.
// `TokEnd`: The position in the buffer where the token ends.
// `Kind`: The kind of token being formed.
void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind) {
  // Set the token's kind (type).
  Tok.Kind = Kind;

  // Set the token's text by creating a string reference from `BufferPtr` to `TokEnd`.
  Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);

  // Advance the lexer buffer pointer to `TokEnd` so that the next token starts after this one.
  BufferPtr = TokEnd;
}

