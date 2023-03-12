#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "object.h"
#include "program.h"
#include "thusly_value.h"
#include "tokenizer.h"

#ifdef DEBUG_COMPILATION
#include "debug.h"
#endif

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
  PRECEDENCE_EQUALITY,
  PRECEDENCE_COMPARISON,
  PRECEDENCE_TERM,
  PRECEDENCE_FACTOR,
  PRECEDENCE_UNARY,
} Precedence;

/// A function used for parsing an expression.
typedef void (*ParseFunction)(Parser*);

/// Holds the function used for parsing a prefix or infix expression based
/// on the given precedence level.
typedef struct {
  ParseFunction prefix;
  ParseFunction infix;
  Precedence precedence;
} ParseRule;

static void parse_binary(Parser* parser);
static void parse_boolean(Parser* parser);
static void parse_grouping(Parser* parser);
static void parse_none(Parser* parser);
static void parse_number(Parser* parser);
static void parse_text(Parser* parser);
static void parse_unary(Parser* parser);

/// The parse rules associated with each type of token.
static ParseRule rules[] = {
  // Format (the token type enum constant becomes the index):
  // [TOKEN_TYPE]               = { PREFIX_RULE, INFIX_RULE, PRECEDENCE_LEVEL }

  // Punctuation and non-keyword operators
  [TOKEN_CLOSE_PAREN]           = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_COLON]                 = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_EQUALS]                = { NULL, parse_binary, PRECEDENCE_EQUALITY },
  [TOKEN_EXCLAMATION_EQUALS]    = { NULL, parse_binary, PRECEDENCE_EQUALITY },
  [TOKEN_GREATER_THAN]          = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_GREATER_THAN_EQUALS]   = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_LESS_THAN]             = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_LESS_THAN_EQUALS]      = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_MINUS]                 = { parse_unary, parse_binary, PRECEDENCE_TERM },
  [TOKEN_OPEN_PAREN]            = { parse_grouping, NULL, PRECEDENCE_IGNORE },
  [TOKEN_PLUS]                  = { NULL, parse_binary, PRECEDENCE_TERM },
  [TOKEN_SLASH]                 = { NULL, parse_binary, PRECEDENCE_FACTOR },
  [TOKEN_STAR]                  = { NULL, parse_binary, PRECEDENCE_FACTOR },

  // Reserved keywords
  [TOKEN_AND]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_FALSE]                 = { parse_boolean, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NONE]                  = { parse_none, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NOT]                   = { parse_unary, NULL, PRECEDENCE_IGNORE },
  [TOKEN_OR]                    = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_PRINT]                 = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_TRUE]                  = { parse_boolean, NULL, PRECEDENCE_IGNORE },
  [TOKEN_VAR]                   = { NULL, NULL, PRECEDENCE_IGNORE },

  // Literals
  [TOKEN_IDENTIFIER]            = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NUMBER]                = { parse_number, NULL, PRECEDENCE_IGNORE },
  [TOKEN_TEXT]                  = { parse_text, NULL, PRECEDENCE_IGNORE },

  // Formatting
  [TOKEN_FILE_END]              = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NEWLINE]               = { NULL, NULL, PRECEDENCE_IGNORE },

  // Errors
  [TOKEN_LEXICAL_ERROR]         = { NULL, NULL, PRECEDENCE_IGNORE },
};

static ParseRule* get_rule(TokenType type) {
  return &rules[type];
}

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

static void parse_precedence(Parser* parser, Precedence min_precedence) {
  advance(parser);

  // All valid expressions must begin with a prefix token.
  ParseFunction prefix_rule = get_rule(parser->previous.type)->prefix;
  if (prefix_rule == NULL) {
    error(parser, "Expected an expression.");
    return;
  }
  prefix_rule(parser);

  // Keep parsing expressions as long as the precedence level is high enough.
  while (get_rule(parser->current.type)->precedence >= min_precedence) {
    advance(parser);
    ParseFunction infix_rule = get_rule(parser->previous.type)->infix;
    infix_rule(parser);
  }

  if (parser->current.type == TOKEN_NEWLINE)
    advance(parser);
}

static void parse_expression(Parser* parser) {
  // Lowest precedence = PRECEDENCE_ASSIGNMENT
  parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
}

static void parse_binary(Parser* parser) {
  TokenType operator = parser->previous.type;
  ParseRule* rule = get_rule(operator);
  // Parse the right-hand operand using one precedence level above the current one
  // in order to enforce left-associativity. E.g.:
  // 5 + 6 + 7 should be parsed (5 + 6) + 7
  // 5 + 6 * 7 / 8 should be parsed 5 + ((6 * 7) / 8)
  parse_precedence(parser, (Precedence)(rule->precedence + 1));

  switch (operator) {
    case TOKEN_EQUALS:
      write_instruction(parser, OP_EQUALS);
      break;
    case TOKEN_EXCLAMATION_EQUALS:
      write_instruction(parser, OP_NOT_EQUALS);
      break;
    case TOKEN_GREATER_THAN:
      write_instruction(parser, OP_GREATER_THAN);
      break;
    case TOKEN_GREATER_THAN_EQUALS:
      write_instruction(parser, OP_GREATER_THAN_EQUALS);
      break;
    case TOKEN_LESS_THAN:
      write_instruction(parser, OP_LESS_THAN);
      break;
    case TOKEN_LESS_THAN_EQUALS:
      write_instruction(parser, OP_LESS_THAN_EQUALS);
      break;
    case TOKEN_PLUS:
      write_instruction(parser, OP_ADD);
      break;
    case TOKEN_MINUS:
      write_instruction(parser, OP_SUBTRACT);
      break;
    case TOKEN_STAR:
      write_instruction(parser, OP_MULTIPLY);
      break;
    case TOKEN_SLASH:
      write_instruction(parser, OP_DIVIDE);
      break;
    default:
      // This should not be reachable.
      return;
  }
}

static void parse_boolean(Parser* parser) {
  switch (parser->previous.type) {
    case TOKEN_FALSE:
      write_instruction(parser, OP_CONSTANT_FALSE);
      break;
    case TOKEN_TRUE:
      write_instruction(parser, OP_CONSTANT_TRUE);
      break;
    default:
      // This should not be reachable.
      return;
  }
}

static void parse_grouping(Parser* parser) {
  parse_expression(parser);
  consume(parser, TOKEN_CLOSE_PAREN, "A closing parenthesis ')' is missing.");
}

static void parse_none(Parser* parser) {
  write_instruction(parser, OP_CONSTANT_NONE);
}

static void parse_number(Parser* parser) {
  double value = strtod(parser->previous.lexeme, NULL);
  write_constant_instruction(parser, FROM_C_DOUBLE(value));
}

static void parse_text(Parser* parser) {
  write_constant_instruction(
    parser,
    // Copy the lexeme without the surrounding double quotes.
    FROM_C_OBJECT_PTR(copy_c_string(parser->previous.lexeme + 1, parser->previous.length - 2))
  );
}

static void parse_unary(Parser* parser) {
  TokenType operator = parser->previous.type;
  parse_precedence(parser, PRECEDENCE_UNARY);

  switch (operator) {
    case TOKEN_MINUS:
      write_instruction(parser, OP_NEGATE);
      break;
    case TOKEN_NOT:
      write_instruction(parser, OP_NOT);
      break;
    default:
      // This should not be reachable.
      return;
  }
}

static void end_compilation(Parser* parser) {
  write_return_instruction(parser);

  #ifdef DEBUG_COMPILATION
    if (!parser->has_error)
      disassemble_program(get_writable_program(parser), "Program");
  #endif
}

bool compile(const char* source, Program* out_program) {
  Parser parser;
  init_parser(&parser, out_program);
  init_tokenizer(&parser.tokenizer, source);

  advance(&parser);
  parse_expression(&parser);
  consume(&parser, TOKEN_FILE_END, "Expected the end of an expression.");
  end_compilation(&parser);

  return !parser.has_error;
}
