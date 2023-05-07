#ifndef CTHUSLY_TOKENIZER_H
#define CTHUSLY_TOKENIZER_H

#include "common.h"

// Some of the token types (incrementally added).

typedef enum {
  // Punctuation and non-keyword operators
  TOKEN_CLOSE_PAREN,
  TOKEN_COLON,
  // TOKEN_COMMA,
  // TOKEN_DOT,
  TOKEN_EQUALS,
  TOKEN_EXCLAMATION_EQUALS,
  TOKEN_GREATER_THAN,
  TOKEN_GREATER_THAN_EQUALS,
  TOKEN_LESS_THAN,
  TOKEN_LESS_THAN_EQUALS,
  TOKEN_MINUS,
  TOKEN_OPEN_PAREN,
  TOKEN_PLUS,
  TOKEN_SLASH,
  TOKEN_STAR,

  // Reserved keywords
  TOKEN_AND,
  // TOKEN_CONST,
  // TOKEN_ELSE,
  // TOKEN_END,
  TOKEN_FALSE,
  // TOKEN_FUN,
  // TOKEN_IF,
  TOKEN_MOD,
  TOKEN_NONE,
  TOKEN_NOT,
  TOKEN_OR,
  TOKEN_PRINT,    // Temporary until built-in function exists
  // TOKEN_RETURN,
  TOKEN_TRUE,
  TOKEN_VAR,

  // Literals
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER,
  TOKEN_TEXT,

  // Formatting
  // TOKEN_BLOCK,
  TOKEN_FILE_END,
  TOKEN_NEWLINE,

  // Errors
  TOKEN_LEXICAL_ERROR,
} TokenType;

typedef struct {
  TokenType type;
  // This points into the original source (thus, the source
  // is freed only after it has finished being interpreted).
  const char* lexeme;
  int length;
  int line;
} Token;

typedef struct {
  const char* start;
  const char* current;
  int line;
  bool is_blank_line;
} Tokenizer;

void tokenizer_init(Tokenizer* tokenizer, const char* source);
Token tokenize(Tokenizer* tokenizer);

#endif
