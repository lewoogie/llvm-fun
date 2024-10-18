#ifndef LEXER_H
#define LEXER_H

// Including LLVM's utility classes for efficient string manipulation and memory management.
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

// Forward declaration of the Lexer class, allowing Token to declare Lexer as a friend.
class Lexer;

// Token class represents a single lexical token from the input.
class Token {
  // Declaring Lexer as a friend class so that Lexer can access private members of Token.
  friend class Lexer;

public:
  // Enumeration of different types of tokens that can be recognized by the lexer.
  enum TokenKind : unsigned short {
    eoi,       // End of input
    unknown,   // Unknown token (typically when the lexer encounters an unexpected character)
    ident,     // Identifier (like variable names)
    number,    // Numeric literals
    comma,     // ','
    colon,     // ':'
    plus,      // '+'
    minus,     // '-'
    star,      // '*'
    slash,     // '/'
    l_paren,   // '('
    r_paren,   // ')'
    KW_with    // Keyword "with"
  };

private:
  TokenKind Kind;           // The kind/type of the token (e.g., ident, number, plus, etc.).
  llvm::StringRef Text;      // The actual text/substring corresponding to this token from the input.

public:
  // Getter for the type of the token.
  TokenKind getKind() const { return Kind; }

  // Getter for the text of the token, returning a reference to the string data.
  llvm::StringRef getText() const {
    return Text;
  }

  // Utility function to check if the token is of a specific kind.
  bool is(TokenKind K) const { return Kind == K; }

  // Utility function to check if the token is one of two specific kinds.
  bool isOneOf(TokenKind K1, TokenKind K2) const {
    return is(K1) || is(K2);
  }

  // Variadic template function to check if the token matches any of several kinds.
  // It works recursively, reducing to the previous `isOneOf` function when no more arguments remain.
  template <typename... Ts>
  bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks) const {
    return is(K1) || isOneOf(K2, Ks...);
  }
};

// Lexer class is responsible for processing the input buffer and generating tokens.
class Lexer {
  const char *BufferStart;  // Points to the start of the input buffer.
  const char *BufferPtr;    // Points to the current position in the input buffer being processed.

public:
  // Constructor for Lexer, taking the input buffer (string) and initializing pointers.
  Lexer(const llvm::StringRef &Buffer) {
    BufferStart = Buffer.begin();  // Set the start of the buffer.
    BufferPtr = BufferStart;       // Initialize the current position to the start of the buffer.
  }

  // Function to generate the next token from the input and store it in `token`.
  void next(Token &token);

private:
  // Utility function to create and set a token based on the current input.
  // `Result` is the token to set, `TokEnd` is the end of the token in the buffer, and `Kind` is the type of the token.
  void formToken(Token &Result, const char *TokEnd,
                 Token::TokenKind Kind);
};

#endif  // End of header guard

