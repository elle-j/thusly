#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"

// TODO: Perhaps move this struct to .h
typedef struct {
  Token current;
  Token previous;
  bool has_error;
  bool panic_mode;
} Parser;

static void init_parser(Parser* parser) {
  parser->has_error = false;
  parser->panic_mode = false;
}

static void error_at(Parser* parser, Token* token, const char* message) {
  // If the parser is in panic mode, return here in order to not report
  // potentially false and unhelpful cascaded errors.
  if (parser->panic_mode)
    return;

  parser->panic_mode = true;
  parser->has_error = true;

  fprintf(stderr, "ERROR on line %d", token->line);
  if (token->type == TOKEN_FILE_END)
    fprintf(stderr, " at the end of the file.");
  else if (token->type == TOKEN_LEXICAL_ERROR) {
    // The error message for `TOKEN_LEXICAL_ERROR`s has been passed in as
    // the `message` via `advance()`. There's no need to do anything here.
  }
  else {
    // `token->length` is passed as the argument for '*' (determining how many
    // characters of `token->lexeme` to display). E.g. tokentype[13] lexeme[and]'.
    fprintf(stderr, " at '%.*s'", token->length, token->lexeme);
  }

  fprintf(stderr, ":\n\t>> Help: %s\n", message);
}

static void error(Parser* parser, const char* message) {
  error_at(parser, &parser->previous, message);
}

static bool check(Parser* parser, TokenType type) {
  return parser->current.type == type;
}

static void advance(Parser* parser, Tokenizer* tokenizer) {
  parser->previous = parser->current;
  parser->current = tokenize(tokenizer);

  // Whenever the tokenizer encounters an error it produces a token of type
  // `TOKEN_LEXICAL_ERROR`. Loop past (and report) these until the next valid token.
  while (parser->current.type == TOKEN_LEXICAL_ERROR) {
    // Lexical error tokens store the error message on the lexeme field.
    error_at(parser, &parser->current, parser->current.lexeme);
    parser->current = tokenize(tokenizer);
  }
}

static void consume(Parser* parser, Tokenizer* tokenizer, TokenType type, const char* err_message) {
  if (check(parser, type)) {
    advance(parser, tokenizer);
    return;
  }

  error_at(parser, &parser->current, err_message);
}

bool compile(VM* vm, const char* source, Program* out_program) {
  Tokenizer tokenizer;
  init_tokenizer(&tokenizer, source);
  Parser parser;
  init_parser(&parser);

  advance(&parser, &tokenizer);

  printf("Token: %.*s\n", parser.current.length, parser.current.lexeme);

  return !parser.has_error;
}
