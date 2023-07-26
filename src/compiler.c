#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "gc_object.h"
#include "program.h"
#include "thusly_value.h"
#include "tokenizer.h"

#ifdef DEBUG_COMPILATION
#include "debug.h"
#endif

#define NOT_FOUND (-1)

/// The compiler and parser - parses the tokens received by the tokenizer on demand
/// (it controls the tokenizer) and writes the bytecode instructions for the VM in
/// a single pass in the instruction format expected by the VM. (It performs top-down
/// operator precedence parsing.)

/// A user-defined variable declared in the source code.
typedef struct {
  Token name;
  // The depth/level at which the variable was declared.
  int depth;
} Variable;

typedef struct {
  // When a variable is declared in the source code, it gets added to this array.
  // The order will coincide with how they end up on the VM's stack. Due to only
  // supporting instructions using 1 byte for operands, the array count cannot exceed 256.
  Variable variables[UINT8_MAX + 1];
  // Number of variables currently in scope.
  int variable_count;
  // The current level of nesting (number of surrounding blocks).
  int scope_depth;
} Compiler;

typedef struct {
  Compiler* compiler;
  Environment* environment;
  Program* writable_program; // TODO: Modify
  Tokenizer tokenizer;
  Token current;
  Token previous;
  bool saw_error;
  bool panic_mode;
} Parser;

/// The levels of precedence from lowest to highest.
typedef enum {
  PRECEDENCE_IGNORE,
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
typedef void (*ParseFunction)(Parser*, bool is_assignable);

/// Holds the function used for parsing a prefix or infix expression based
/// on the given precedence level.
typedef struct {
  ParseFunction prefix;
  ParseFunction infix;
  Precedence precedence;
} ParseRule;

static void parse_statement(Parser* parser);
static void parse_block_statement(Parser* parser);
static void parse_expression_statement(Parser* parser);
static void parse_out_statement(Parser* parser);
static void parse_var_statement(Parser* parser);

static void parse_expression(Parser* parser);
static void parse_binary(Parser* parser, bool _);
static void parse_boolean(Parser* parser, bool _);
static void parse_grouping(Parser* parser, bool _);
static void parse_none(Parser* parser, bool _);
static void parse_number(Parser* parser, bool _);
static void parse_text(Parser* parser, bool _);
static void parse_unary(Parser* parser, bool _);
static void parse_variable(Parser* parser, bool is_assignable);

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
  [TOKEN_BLOCK]                 = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_END]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_FALSE]                 = { parse_boolean, NULL, PRECEDENCE_IGNORE },
  [TOKEN_MOD]                   = { NULL, parse_binary, PRECEDENCE_FACTOR },
  [TOKEN_NONE]                  = { parse_none, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NOT]                   = { parse_unary, NULL, PRECEDENCE_IGNORE },
  [TOKEN_OR]                    = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_OUT]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_TRUE]                  = { parse_boolean, NULL, PRECEDENCE_IGNORE },
  [TOKEN_VAR]                   = { NULL, NULL, PRECEDENCE_IGNORE },

  // Literals
  [TOKEN_IDENTIFIER]            = { parse_variable, NULL, PRECEDENCE_IGNORE },
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

static void compiler_init(Compiler* compiler) {
  compiler->variable_count = 0;
  compiler->scope_depth = 0;
}

static void parser_init(Parser* parser, Compiler* compiler, Environment* environment, Program* writable_program) {
  parser->compiler = compiler;
  parser->environment = environment;
  parser->writable_program = writable_program;
  parser->saw_error = false;
  parser->panic_mode = false;
}

static Program* get_writable_program(Parser* parser) {
  return parser->writable_program;
}

static void error_at(Parser* parser, Token* token, const char* message) {
  // If the parser is in panic mode, return here in order to not report
  // potentially false and unhelpful cascaded errors.
  if (parser->panic_mode)
    return;

  parser->panic_mode = true;
  parser->saw_error = true;

  fprintf(stderr, "\n---------");
  fprintf(stderr, "\n| error |");
  fprintf(stderr, "\n---------");
  fprintf(stderr, "\n\t> Line:\n\t\t%d", token->line);
  fprintf(stderr, "\n\t> Where:\n\t\t");
  if (token->type == TOKEN_FILE_END)
    fprintf(stderr, "At the end of the file");
  else if (token->type == TOKEN_NEWLINE)
    fprintf(stderr, "At the end of the line");
  else if (token->type == TOKEN_LEXICAL_ERROR) {
    // The error message for `TOKEN_LEXICAL_ERROR` was stored as the `lexeme`
    // and has been passed in as the `message` via `advance()`. Therefore,
    // `parser.tokenizer` is being accessed directly here instead.
    int token_length = parser->tokenizer.current - parser->tokenizer.start;
    fprintf(stderr, "At '%.*s'", token_length, parser->tokenizer.start);
  }
  else
    fprintf(stderr, "At '%.*s'", token->length, token->lexeme);

  fprintf(stderr, "\n\t> What's wrong:\n\t\t%s\n", message);
}

static void error(Parser* parser, const char* message) {
  error_at(parser, &parser->previous, message);
}

/// Compare the type of the current token being parsed with a given type.
static bool compare(Parser* parser, TokenType type) {
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

/// Compare the type of the current token being parsed with the expected type,
/// and advance if they match or generate an error.
static void consume(Parser* parser, TokenType expected_type, const char* err_message) {
  if (compare(parser, expected_type)) {
    advance(parser);
    return;
  }

  error_at(parser, &parser->current, err_message);
}

static void consume_newline(Parser* parser) {
  if (compare(parser, TOKEN_NEWLINE)) {
    advance(parser);
    return;
  }

  error_at(parser, &parser->current, "The statement must end with a newline.");
}

/// Compare the type of the current token being parsed with a given type,
/// and advance the parser if they match.
static bool match(Parser* parser, TokenType type) {
  if (!compare(parser, type))
    return false;

  advance(parser);

  return true;
}

static bool is_at_end(Parser* parser) {
  return parser->current.type == TOKEN_FILE_END;
}

static bool is_at_start_of_statement(Parser* parser) {
  switch (parser->current.type) {
    // TODO: Add synchronization points as these are added to the language.
    case TOKEN_OUT:
    case TOKEN_VAR:
      return true;
    default:
      return false;
  }
}

/// Synchronize the parsing when the parser has entered panic mode. This
/// synchronizes to the next statement encountered and is performed in order
/// to minimize cascaded and falsely reported errors.
static void synchronize(Parser* parser) {
  parser->panic_mode = false;

  while (!is_at_end(parser) && !is_at_start_of_statement(parser)) {
    advance(parser);
  }
}

static byte make_constant(Parser* parser, ThuslyValue value) {
  unsigned int constant_index = program_add_constant(get_writable_program(parser), value);
  // The operand to the OP_CONSTANT instruction (i.e. the index of the constant)
  // currently only supports 1 byte.
  if (constant_index > UINT8_MAX) {
    error(parser, "Too many constants have been used.");
    return 0;
  }

  return (byte)constant_index;
}

static byte make_identifier_constant(Parser* parser, Token* token) {
  return make_constant(
    parser,
    FROM_C_OBJECT_PTR(
      copy_c_string(parser->environment, token->lexeme, token->length)
    )
  );
}

static void write_instruction(Parser* parser, byte instruction) {
  program_write(get_writable_program(parser), instruction, parser->previous.line);
}

static void write_instructions(Parser* parser, byte instruction1, byte instruction2) {
  write_instruction(parser, instruction1);
  write_instruction(parser, instruction2);
}

static void write_constant_instruction(Parser* parser, ThuslyValue value) {
  write_instructions(parser, OP_CONSTANT, make_constant(parser, value));
}

static void write_return_instruction(Parser* parser) {
  write_instruction(parser, OP_RETURN);
}

static bool is_in_innermost_scope(Parser* parser, Variable* variable) {
  return variable->depth == parser->compiler->scope_depth;
}

static void create_scope(Parser* parser) {
  parser->compiler->scope_depth++;
}

/// Discard the innermost scope along with the variables declared there.
static void discard_scope(Parser* parser) {
  Compiler* compiler = parser->compiler;

  // Since variables are added as they appear in the source code, the variables
  // in the innermost scope exist at the end of the array. When the depth of a
  // variable no longer is the same as the current scope, all variables in the
  // innermost scope have been discarded.
  while (compiler->variable_count > 0 &&
         is_in_innermost_scope(parser, &compiler->variables[compiler->variable_count - 1])) {
    // TODO: Add OP_POPN instruction.
    write_instruction(parser, OP_POP);
    compiler->variable_count--;
  }

  // No need to check if it is in the global scope before decrementing.
  // `create_scope()` will only be called when a block is encountered.
  // Thus, `discard_scope()` will only be called when depth > 0.
  compiler->scope_depth--;
}

static bool is_same_name(Token* first, Token* second) {
  if (first->length != second->length)
    return false;

  return memcmp(first->lexeme, second->lexeme, first->length) == 0;
}

static void add_variable(Parser* parser, Token name) {
  bool has_reached_max_variables = parser->compiler->variable_count == UINT8_MAX + 1;
  if (has_reached_max_variables) {
    error(parser, "Too many variables are currently in scope.");
    return;
  }

  Variable* variable = &parser->compiler->variables[parser->compiler->variable_count++];
  variable->name = name;
  variable->depth = parser->compiler->scope_depth;
}

static void declare_variable(Parser* parser) {
  // All user-defined variables are resolved at compile time.
  Token* name = &parser->previous;
  for (int i = parser->compiler->variable_count - 1; i >= 0; i--) {
    Variable* existing_variable = &parser->compiler->variables[i];
    bool is_declared_in_different_scope =
      existing_variable->depth != NOT_FOUND &&
      existing_variable->depth < parser->compiler->scope_depth;

    if (is_declared_in_different_scope)
      break;

    if (is_same_name(name, &existing_variable->name))
      error(parser, "A variable with the same name has already been declared in this scope.");
  }

  add_variable(parser, *name);
}

static void define_variable(Parser* parser) {
  // TODO
}

/// Resolve a variable by returning the location on the VM's stack that
/// the variable's value exists. The value coincides with the variable
/// that was declared lexically closest to where it is being accessed,
/// going from the innermost scope outward.
static int resolve(Parser* parser, Token* name) {
  for (int i = parser->compiler->variable_count - 1; i >= 0; i--) {
    Variable* existing_variable = &parser->compiler->variables[i];
    if (is_same_name(name, &existing_variable->name))
      // The order and position of the `variables` array will be
      // identical to how they end up on the stack.
      return i;
  }

  return NOT_FOUND;
}

static void access_or_assign_variable(Parser* parser, Token name, bool is_assignable) {
  int stack_slot = resolve(parser, &name);
  if (stack_slot == NOT_FOUND) {
    error_at(parser, &name, "The variable has not been declared. Use 'var <name> : <value>' to declare it first.");
    // return;
  }

  if (is_assignable && match(parser, TOKEN_COLON)) {
    parse_expression(parser);
    write_instructions(parser, OP_SET_VAR, (byte)stack_slot);
  }
  else
    write_instructions(parser, OP_GET_VAR, (byte)stack_slot);
}

static void parse_statement(Parser* parser) {
  if (match(parser, TOKEN_VAR))
    parse_var_statement(parser);
  else if (match(parser, TOKEN_OUT))
    parse_out_statement(parser);
  else if (match(parser, TOKEN_BLOCK)) {
    create_scope(parser);
    parse_block_statement(parser);
    discard_scope(parser);
  }
  else
    parse_expression_statement(parser);

  if (parser->panic_mode)
    synchronize(parser);
}

static void parse_block_statement(Parser* parser) {
  consume_newline(parser);
  while (!compare(parser, TOKEN_END) && !is_at_end(parser))
    parse_statement(parser);

  consume(parser, TOKEN_END, "The block has not been terminated. Use 'end' at the end of the block.");
  consume_newline(parser);
}

static void parse_expression_statement(Parser* parser) {
  parse_expression(parser);
  consume_newline(parser);
  write_instruction(parser, OP_POP);
}

static void parse_out_statement(Parser* parser) {
  parse_expression(parser);
  consume_newline(parser);
  write_instruction(parser, OP_OUT);
}

static void parse_var_statement(Parser* parser) {
  consume(parser, TOKEN_IDENTIFIER, "A name for the variable is missing.");
  declare_variable(parser);

  consume(parser, TOKEN_COLON, "The variable is missing an initializer. Use ':' to initialize it with a value.");
  parse_expression(parser);
  consume_newline(parser);
  define_variable(parser);
}

static void parse_precedence(Parser* parser, Precedence min_precedence) {
  advance(parser);

  // All valid expressions must begin with a prefix token.
  ParseFunction prefix_rule = get_rule(parser->previous.type)->prefix;
  if (prefix_rule == NULL) {
    error(parser, "You must provide an expression.");
    return;
  }

  // Assignments have the lowest precedence of the expressions; thus, if
  // an assignment is encountered, it should only continue parsing it if
  // the surrounding expression is of the same or lower precedence.
  // - Examples of when to continue parsing:
  //   x : y : 1        // Expected parsing:  x : (y : 1)
  //   x + (y : z + 1)  // Expected parsing:  x + (y : (z + 1))
  // - Examples of when not to continue parsing:
  //   x + y : 1        // '+' has higher precedence. Expected parsing:  (x + y) : 1
  //   -x : 1           // '-' has higher precedence. Expected parsing:  (-x) : 1
  bool is_assignable = compare(parser, TOKEN_COLON) && min_precedence <= PRECEDENCE_ASSIGNMENT;
  prefix_rule(parser, is_assignable);

  // Keep parsing expressions as long as the precedence level is high enough.
  while (get_rule(parser->current.type)->precedence >= min_precedence) {
    advance(parser);
    ParseFunction infix_rule = get_rule(parser->previous.type)->infix;
    infix_rule(parser, is_assignable);
  }

  // If there was an assignment but the precedence of the surrounding expression
  // was too high, the assignment and its corresponding `:` will not have been
  // parsed by `access_or_assign_variable()`. This means the target was invalid.
  // Generate the error after the `prefix_rule()` call in order for resolution
  // errors to be displayed first.
  if (!is_assignable && match(parser, TOKEN_COLON)) {
    error(parser, "You are trying to assign a value to an invalid target.");
  }
}

static void parse_expression(Parser* parser) {
  // Lowest precedence = PRECEDENCE_ASSIGNMENT
  parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
}

static void parse_binary(Parser* parser, bool _) {
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
    case TOKEN_MOD:
      write_instruction(parser, OP_MODULO);
      break;
    default:
      // This should not be reachable.
      return;
  }
}

static void parse_boolean(Parser* parser, bool _) {
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

static void parse_grouping(Parser* parser, bool _) {
  parse_expression(parser);
  consume(parser, TOKEN_CLOSE_PAREN, "A closing parenthesis ')' is missing.");
}

static void parse_none(Parser* parser, bool _) {
  write_instruction(parser, OP_CONSTANT_NONE);
}

static void parse_number(Parser* parser, bool _) {
  double value = strtod(parser->previous.lexeme, NULL);
  write_constant_instruction(parser, FROM_C_DOUBLE(value));
}

static void parse_text(Parser* parser, bool _) {
  write_constant_instruction(
    parser,
    FROM_C_OBJECT_PTR(
      // Copy the lexeme without the surrounding double quotes.
      copy_c_string(parser->environment, parser->previous.lexeme + 1, parser->previous.length - 2)
    )
  );
}

static void parse_unary(Parser* parser, bool _) {
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

static void parse_variable(Parser* parser, bool is_assignable) {
  access_or_assign_variable(parser, parser->previous, is_assignable);
}

static void end_compilation(Parser* parser) {
  write_return_instruction(parser);

  #ifdef DEBUG_COMPILATION
    if (!parser->saw_error)
      disassemble_program(get_writable_program(parser), "Program");
  #endif
}

bool compile(Environment* environment, const char* source, Program* out_program) {
  Parser parser;
  Compiler compiler;
  compiler_init(&compiler);
  parser_init(&parser, &compiler, environment, out_program);
  // TODO: Refactor this into a call in `parser_init`.
  tokenizer_init(&parser.tokenizer, source);

  advance(&parser);

  while (!match(&parser, TOKEN_FILE_END)) {
    parse_statement(&parser);
  }

  end_compilation(&parser);

  return !parser.saw_error;
}
