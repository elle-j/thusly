#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "program.h"
#include "thusly_value.h"
#include "tokenizer.h"

/// The compiler and parser - parses the tokens received by the tokenizer on demand
/// (it controls the tokenizer) and writes the bytecode instructions for the VM in
/// a single pass in the instruction format expected by the VM. (It performs top-down
/// operator precedence parsing.)

///
typedef struct {
  Program* writable_program; // TODO: Modify
  Tokenizer tokenizer;
  Token current;
  Token previous;
  bool has_error;
  bool panic_mode;
} Parser;

/// The levels of precedence from lowest to highest.
typedef enum {
  PRECEDENCE_IGNORE,      // E.g. `(`, `,`, or non-operator keywords.
  PRECEDENCE_ASSIGNMENT,
  PRECEDENCE_DISJUNCTION,
  PRECEDENCE_CONJUNCTION,
  PRECEDENCE_EQUALITY,    // TODO: Perhaps combine EQUALITY and COMPARISON
  PRECEDENCE_COMPARISON,
  PRECEDENCE_TERM,
  PRECEDENCE_FACTOR,
  PRECEDENCE_UNARY,
} Precedence;

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
    fprintf(stderr, " at the end of the file");
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

static void write_instruction(Parser* parser, byte instruction) {
  append_instruction(get_writable_program(parser), instruction, parser->previous.line);
}

static void write_instructions(Parser* parser, byte instruction1, byte instruction2) {
  write_instruction(parser, instruction1);
  write_instruction(parser, instruction2);
}

static byte make_constant(Parser* parser, ThuslyValue value) {
  unsigned int constant_index = add_constant(get_writable_program(parser), value);
  // The operand to the OP_CONSTANT instruction (i.e. the index of the constant)
  // currently only supports 1 byte.
  if (constant_index > UINT8_MAX) {
    error(parser, "Too many constants have been used.");
    return 0;
  }

  return (byte)constant_index;
}

static void write_constant_instruction(Parser* parser, ThuslyValue value) {
  write_instructions(parser, OP_CONSTANT, make_constant(parser, value));
}

static void write_return_instruction(Parser* parser) {
  write_instruction(parser, OP_RETURN);
}

static void parse_precedence(Parser* parser, Precedence precedence) {
  // TODO
}

static void parse_expression(Parser* parser) {
  // Lowest precedence = PRECEDENCE_ASSIGNMENT
  parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
}

static void parse_grouping(Parser* parser) {
  parse_expression(parser);
  consume(parser, TOKEN_CLOSE_PAREN, "A closing parenthesis ')' is missing.");
}

static void parse_number(Parser* parser) {
  double value = strtod(parser->previous.lexeme, NULL);
  write_constant_instruction(parser, value);
}

static void parse_unary(Parser* parser) {
  TokenType operator = parser->previous.type;
  parse_precedence(parser, PRECEDENCE_UNARY);

  switch (operator) {
    case TOKEN_MINUS:
      write_instruction(parser, OP_NEGATE);
      break;
  }
}

static void end_compilation(Parser* parser) {
  write_return_instruction(parser);
}

bool compile(const char* source, Program* out_program) {
  Parser parser;
  init_parser(&parser, out_program);
  init_tokenizer(&parser.tokenizer, source);

  advance(&parser);
  advance(&parser);
  parse_expression(&parser);
  parse_number(&parser);
  consume(&parser, TOKEN_FILE_END, "Expected the end of an expression.");
  end_compilation(&parser);

  return !parser.has_error;
}
