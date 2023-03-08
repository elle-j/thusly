#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"

typedef struct {
  Program* writable_program; // TODO: Modify
  Tokenizer tokenizer;
  Token current;
  Token previous;
  bool has_error;
  bool panic_mode;
} Parser;

static Program* get_writable_program(Parser* parser) {
  return parser->writable_program;
}

static void init_parser(Parser* parser, Program* writable_program) {
  parser->writable_program = writable_program;
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
    // The error message for `TOKEN_LEXICAL_ERROR` has been passed in as
    // the `message` via `advance()`. There's no need to do anything here.
  }
  else
    fprintf(stderr, " at '%.*s'", token->length, token->lexeme);

  fprintf(stderr, ":\n\t>> Help: %s\n", message);
}

static void error(Parser* parser, const char* message) {
  error_at(parser, &parser->previous, message);
}

static bool check(Parser* parser, TokenType type) {
  return parser->current.type == type;
}

static void advance(Parser* parser) {
  parser->previous = parser->current;
  parser->current = tokenize(&parser->tokenizer);

  // Whenever the tokenizer encounters an error it produces a token of type
  // `TOKEN_LEXICAL_ERROR`. Loop past (and report) these until the next valid token.
  while (parser->current.type == TOKEN_LEXICAL_ERROR) {
    // Lexical error tokens store the error message on the lexeme field.
    error_at(parser, &parser->current, parser->current.lexeme);
    parser->current = tokenize(&parser->tokenizer);
  }
}

static void consume(Parser* parser, TokenType type, const char* err_message) {
  if (check(parser, type)) {
    advance(parser);
    return;
  }

  error_at(parser, &parser->current, err_message);
}

bool compile(const char* source, Program* out_program) {
  Parser parser;
  init_parser(&parser, out_program);
  init_tokenizer(&parser.tokenizer, source);

  advance(&parser);
  consume(&parser, TOKEN_FILE_END, "Expected the end of an expression.");

  return !parser.has_error;
}
