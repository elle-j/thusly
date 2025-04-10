#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "gc_object.h"
#include "program.h"
#include "thusly_value.h"
#include "tokenizer.h"

#ifdef DEBUG_MODE
#include "debug.h"
#endif

#define VARIABLES_MAX (UINT8_MAX + 1)
#define CONSTANTS_MAX (UINT8_MAX + 1)
#define JUMP_MAX UINT16_MAX
#define PLACEHOLDER_JUMP_TARGET 0xff  // Note: Keep the 0xff value!
#define NOT_FOUND (-1)
#define UNINITIALIZED (-1)

/// A user-defined variable declared in the source code.
typedef struct {
  Token name;
  /// The depth/level at which the variable was declared.
  int depth;
} Variable;

/// The compiler and parser - Parses the tokens received by the tokenizer on demand
/// (it controls the tokenizer) and writes the bytecode instructions for the VM in
/// a single pass in the instruction format expected by the VM. (It performs top-down
/// operator precedence parsing.)

typedef struct {
  /// The variables declared in the source code.
  /// When a variable is declared, it gets added to this array. The order will
  /// coincide with how they end up on the VM's stack. Due to only supporting
  /// instructions using 1 byte for operands, the array count cannot exceed 256.
  Variable variables[VARIABLES_MAX];
  /// The number of variables currently in scope.
  int variable_count;
  /// The current level of nesting (number of surrounding blocks).
  int scope_depth;
} Compiler;

// (see docs above)
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
static void parse_foreach_statement(Parser* parser);
static void parse_if_statement(Parser* parser);
static void parse_out_statement(Parser* parser);
static void parse_var_statement(Parser* parser);
static void parse_while_statement(Parser* parser);

static void parse_expression(Parser* parser);
static void parse_and(Parser* parser, bool _);
static void parse_binary(Parser* parser, bool _);
static void parse_boolean(Parser* parser, bool _);
static void parse_grouping(Parser* parser, bool _);
static void parse_none(Parser* parser, bool _);
static void parse_number(Parser* parser, bool _);
static void parse_or(Parser* parser, bool _);
static void parse_text(Parser* parser, bool _);
static void parse_unary(Parser* parser, bool _);
static void parse_variable(Parser* parser, bool is_assignable);

/// The parse rules associated with each type of token.
static ParseRule rules[] = {
  // Format (the token type enum constant becomes the index):
  // [TOKEN_TYPE]               = { PREFIX_RULE, INFIX_RULE, PRECEDENCE_LEVEL }

  // Punctuation and non-keyword operators
  [TOKEN_CLOSE_BRACE]           = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_CLOSE_PAREN]           = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_COLON]                 = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_EQUALS]                = { NULL, parse_binary, PRECEDENCE_EQUALITY },
  [TOKEN_EXCLAMATION_EQUALS]    = { NULL, parse_binary, PRECEDENCE_EQUALITY },
  [TOKEN_GREATER_THAN]          = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_GREATER_THAN_EQUALS]   = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_LESS_THAN]             = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_LESS_THAN_EQUALS]      = { NULL, parse_binary, PRECEDENCE_COMPARISON },
  [TOKEN_MINUS]                 = { parse_unary, parse_binary, PRECEDENCE_TERM },
  [TOKEN_MINUS_COLON]           = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_OPEN_BRACE]            = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_OPEN_PAREN]            = { parse_grouping, NULL, PRECEDENCE_IGNORE },
  [TOKEN_PLUS]                  = { NULL, parse_binary, PRECEDENCE_TERM },
  [TOKEN_PLUS_COLON]            = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_SLASH]                 = { NULL, parse_binary, PRECEDENCE_FACTOR },
  [TOKEN_SLASH_COLON]           = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_STAR]                  = { NULL, parse_binary, PRECEDENCE_FACTOR },
  [TOKEN_STAR_COLON]            = { NULL, NULL, PRECEDENCE_IGNORE },

  // Reserved keywords
  [TOKEN_AND]                   = { NULL, parse_and, PRECEDENCE_CONJUNCTION },
  [TOKEN_BLOCK]                 = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_END]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_ELSE]                  = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_FALSE]                 = { parse_boolean, NULL, PRECEDENCE_IGNORE },
  [TOKEN_FOREACH]               = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_IF]                    = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_MOD]                   = { NULL, parse_binary, PRECEDENCE_FACTOR },
  [TOKEN_NONE]                  = { parse_none, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NOT]                   = { parse_unary, NULL, PRECEDENCE_IGNORE },
  [TOKEN_OR]                    = { NULL, parse_or, PRECEDENCE_DISJUNCTION },
  [TOKEN_OUT]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_TRUE]                  = { parse_boolean, NULL, PRECEDENCE_IGNORE },
  [TOKEN_VAR]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_WHILE]                 = { NULL, NULL, PRECEDENCE_IGNORE },

  // Literals
  [TOKEN_IDENTIFIER]            = { parse_variable, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NUMBER]                = { parse_number, NULL, PRECEDENCE_IGNORE },
  [TOKEN_TEXT]                  = { parse_text, NULL, PRECEDENCE_IGNORE },

  // Formatting
  [TOKEN_EOF]                   = { NULL, NULL, PRECEDENCE_IGNORE },
  [TOKEN_NEWLINE]               = { NULL, NULL, PRECEDENCE_IGNORE },

  // Errors
  [TOKEN_LEXICAL_ERROR]         = { NULL, NULL, PRECEDENCE_IGNORE },
};

/// Get the parse rule associated with the given token type.
static ParseRule* get_rule(TokenType type) {
  return &rules[type];
}

/// Initialize the compiler.
static void compiler_init(Compiler* compiler) {
  compiler->variable_count = 0;
  compiler->scope_depth = 0;
}

/// Initialize the parser.
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

/// Get the offset of the current instruction from the start instruction.
static int get_current_instruction_offset(Parser* parser) {
  return get_writable_program(parser)->count;
}

/// Report a compile time error and put the parser in panic mode.
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
  if (token->type == TOKEN_EOF)
    fprintf(stderr, "At the end of the file.");
  else if (token->type == TOKEN_NEWLINE)
    fprintf(stderr, "At the end of the line.");
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

/// Report a compile time error at the most recently consumed token,
/// and put the parser in panic mode.
static void error(Parser* parser, const char* message) {
  error_at(parser, &parser->previous, message);
}

/// Compare the type of the current token being parsed with a given type.
static bool compare(Parser* parser, TokenType type) {
  return parser->current.type == type;
}

/// Advance to the next valid token.
static void advance(Parser* parser) {
  parser->previous = parser->current;
  parser->current = tokenize(&parser->tokenizer);

  // Whenever the tokenizer encounters an error it produces a token of type
  // `TOKEN_LEXICAL_ERROR`. Loop past these until the next valid token.
  // (Only the first error will be reported since panic mode will kick in.)
  while (parser->current.type == TOKEN_LEXICAL_ERROR) {
    // Lexical error tokens store the error message on the lexeme field.
    error_at(parser, &parser->current, parser->current.lexeme);
    parser->current = tokenize(&parser->tokenizer);
  }
}

/// Consume the current unconsumed token if it matches the expected type,
/// otherwise report a compile time error.
static void consume(Parser* parser, TokenType expected_type, const char* err_message) {
  if (compare(parser, expected_type)) {
    advance(parser);
    return;
  }

  error_at(parser, &parser->current, err_message);
}

/// Consume the current newline.
static void consume_newline(Parser* parser) {
  consume(parser, TOKEN_NEWLINE, "The line has not been terminated. Add a newline at the end of the line.");
}

/// Consume the current end of a block.
static void consume_end_of_block(Parser* parser) {
  consume(parser, TOKEN_END, "The block has not been terminated. Use 'end' at the end of the block.");
  consume_newline(parser);
}

/// Compare the type of the current token with a given type, and advance
/// the parser if they match.
static bool match(Parser* parser, TokenType type) {
  if (!compare(parser, type))
    return false;

  advance(parser);

  return true;
}

/// Check whether all tokens have been parsed.
static bool is_at_end_of_file(Parser* parser) {
  return parser->current.type == TOKEN_EOF;
}

/// Check whether the current token is the start of a statement.
static bool is_at_start_of_statement(Parser* parser) {
  switch (parser->current.type) {
    // TODO: Add synchronization points if new statements are added to the language.
    case TOKEN_VAR:
    case TOKEN_OUT:
    case TOKEN_IF:
    case TOKEN_BLOCK:
    case TOKEN_FOREACH:
    case TOKEN_WHILE:
      return true;
    default:
      return false;
  }
}

/// Check whether the current token is the end of a block.
static bool is_at_end_of_block(Parser* parser) {
  return compare(parser, TOKEN_END);
}

/// Synchronize the parsing when the parser has entered panic mode. This
/// synchronizes to the next statement encountered or to the end of a block
/// and is performed in order to minimize cascaded and falsely reported errors.
static void synchronize(Parser* parser) {
  parser->panic_mode = false;

  while (!(is_at_start_of_statement(parser) || is_at_end_of_block(parser) || is_at_end_of_file(parser))) {
    advance(parser);
  }
}

/// Add a value to the constant pool and get the location.
static byte make_constant(Parser* parser, ThuslyValue value) {
  unsigned int constant_index = program_add_constant(get_writable_program(parser), value);
  // The operand to the OP_CONSTANT instruction (i.e. the index of the constant)
  // currently only supports 1 byte (256 constants).
  if (constant_index > CONSTANTS_MAX - 1) {
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

/// Overwrite a previously written bytecode instruction.
static void overwrite_instruction(Parser* parser, int offset, byte updated_instruction) {
  program_overwrite(get_writable_program(parser), offset, updated_instruction);
}

/// Write a byte of an instruction.
static void write_instruction(Parser* parser, byte instruction) {
  program_write(get_writable_program(parser), instruction, parser->previous.line);
}

/// Write two bytes of an instruction.
static void write_instructions(Parser* parser, byte instruction1, byte instruction2) {
  write_instruction(parser, instruction1);
  write_instruction(parser, instruction2);
}

/// Write an instruction to load a constant.
static void write_constant_instruction(Parser* parser, ThuslyValue value) {
  write_instructions(parser, OP_CONSTANT, make_constant(parser, value));
}

/// Write an instruction to jump to an earlier instruction.
static void write_jump_backward_instruction(Parser* parser, int target_offset) {
  write_instruction(parser, OP_JUMP_BWD);

  // The jump size (the operand for the jump instruction) should be 2 bytes MORE
  // than the size from the current instruction to the target when jumping backward.
  // This is due to that the VM will read the 3-byte jump instruction (1-byte opcode,
  // 2-byte operand) and thereby move the program counter ahead. So, it needs to jump
  // over the jump instruction itself as well.
  int jump_operand_bytes = 2;
  int jump_size = get_current_instruction_offset(parser) - target_offset + jump_operand_bytes;
  if (jump_size > JUMP_MAX)
    error(parser, "The amount of code to jump over is more than what is currently supported.");

  write_instructions(parser, (jump_size >> 8) & PLACEHOLDER_JUMP_TARGET, jump_size & PLACEHOLDER_JUMP_TARGET);
}

/// Write an instruction to jump to a later instruction. This uses a 16-bit
/// placeholder jump offset and returns where that placeholder starts which
/// should be used for backpatching it.
static int write_jump_forward_instruction(Parser* parser, byte instruction) {
  write_instruction(parser, instruction);
  write_instructions(parser, PLACEHOLDER_JUMP_TARGET, PLACEHOLDER_JUMP_TARGET);

  int jump_operand_bytes = 2;
  int placeholder_start = get_current_instruction_offset(parser) - jump_operand_bytes;

  return placeholder_start;
}

/// Backpatch a previously written jump forward instruction by overwriting the
/// placeholder offset with the now-correct jump target offset. This assumes the
/// function is called immediately before the instruction to jump to is written.
static void patch_jump_forward_instruction(Parser* parser, int placeholder_start) {
  // The jump size (the operand for the jump instruction) should be 2 bytes LESS
  // than the size from the current instruction to the target when jumping forward.
  // This is due to that the VM will read the 3-byte jump instruction (1-byte opcode,
  // 2-byte operand) and thereby move the program counter ahead. So, the VM does
  // not need to jump those bytes again.
  int jump_operand_bytes = 2;
  int jump_size = get_current_instruction_offset(parser) - placeholder_start - jump_operand_bytes;
  if (jump_size > JUMP_MAX)
    error(parser, "The amount of code to jump over is more than what is currently supported.");

  overwrite_instruction(parser, placeholder_start, (jump_size >> 8) & PLACEHOLDER_JUMP_TARGET);
  overwrite_instruction(parser, placeholder_start + 1, jump_size & PLACEHOLDER_JUMP_TARGET);
}

/// Write an instruction to return.
static void write_return_instruction(Parser* parser) {
  write_instruction(parser, OP_RETURN);
}

/// Check whether a variable is declared in the innermost scope.
static bool is_in_innermost_scope(Parser* parser, Variable* variable) {
  return variable->depth == parser->compiler->scope_depth;
}

/// Create a new scope.
static void create_scope(Parser* parser) {
  parser->compiler->scope_depth++;
}

/// Discard the innermost scope along with writing instructions to discard the
/// variables declared there.
static void discard_scope(Parser* parser) {
  Compiler* compiler = parser->compiler;

  // Since variables are added as they appear in the source code, the variables
  // in the innermost scope exist at the end of the array. When the depth of a
  // variable no longer is the same as the current scope, all variables in the
  // innermost scope have been discarded.
  int variable_count_before = compiler->variable_count;
  while (compiler->variable_count > 0 &&
         is_in_innermost_scope(parser, &compiler->variables[compiler->variable_count - 1])) {
    compiler->variable_count--;
  }
  int variables_to_discard = variable_count_before - compiler->variable_count;
  if (variables_to_discard > 1)
    // `N` in `POPN` is treated as "the number to pop minus 1" in order to allow
    // popping the maximum number of variables supported on the stack (UINT8_MAX + 1,
    // i.e. 256). Otherwise, casting 256 to a byte results in 0 due to overflow.
    write_instructions(parser, OP_POPN, (byte)(variables_to_discard - 1));
  else if (variables_to_discard == 1)
    write_instruction(parser, OP_POP);

  // No need to check if it is in the global scope before decrementing.
  // `create_scope()` will only be called when a block is encountered.
  // Thus, `discard_scope()` will only be called when depth > 0.
  compiler->scope_depth--;
}

/// Check whether two variables' names are the same.
static bool is_same_name(Token* first, Token* second) {
  if (first->length != second->length)
    return false;

  return memcmp(first->lexeme, second->lexeme, first->length) == 0;
}

/// Add a variable to the compiler's list of declared variables.
/// The variable will initially be marked as "uninitialized". Once its
/// initializer has been compiled, it will be treated as initialized.
static void add_variable(Parser* parser, Token name) {
  bool has_reached_max_variables = parser->compiler->variable_count == VARIABLES_MAX;
  if (has_reached_max_variables) {
    error(parser, "Too many variables are currently in scope.");
    return;
  }

  Variable* variable = &parser->compiler->variables[parser->compiler->variable_count++];
  variable->name = name;
  // When variables are declared, they are only marked as initialized once the
  // entire initializer has been compiled.
  variable->depth = UNINITIALIZED;
}

/// Mark the last variable added as initialized; meaning, the initializer has
/// been compiled. This is denoted by having a depth != UNINITIALIZED.
static void mark_initialized(Parser* parser) {
  Compiler* compiler = parser->compiler;
  compiler->variables[compiler->variable_count - 1].depth = compiler->scope_depth;
}

/// Check whether a declared variable has been initialized; meaning, the initializer
/// has been compiled. This is denoted by having a depth != UNINITIALIZED.
static bool is_initialized(Variable* variable) {
  return variable->depth != UNINITIALIZED;
}

/// Declare a variable in the current scope, or report an error if a
/// variable with the same name already exists in the same scope.
static void declare_variable(Parser* parser) {
  // All user-defined variables are resolved at compile time.
  Token* name = &parser->previous;
  for (int i = parser->compiler->variable_count - 1; i >= 0; i--) {
    Variable* existing_variable = &parser->compiler->variables[i];
    bool is_declared_in_different_scope =
      is_initialized(existing_variable) &&
      existing_variable->depth < parser->compiler->scope_depth;

    if (is_declared_in_different_scope)
      break;

    if (is_same_name(name, &existing_variable->name))
      error(parser, "A variable with the same name has already been declared in this scope.");
  }

  add_variable(parser, *name);
}

/// Define a variable by marking the last variable added as initialized.
static void define_variable(Parser* parser) {
  mark_initialized(parser);
}

/// Resolve a variable by returning the location on the VM's stack that
/// the variable's value exists. The value coincides with the variable
/// that was declared lexically closest to where it is being accessed,
/// going from the innermost scope outward.
static int resolve(Parser* parser, Token* name) {
  for (int i = parser->compiler->variable_count - 1; i >= 0; i--) {
    Variable* existing_variable = &parser->compiler->variables[i];
    if (is_same_name(name, &existing_variable->name)) {
      // When variables are declared, they are only marked as initialized once the
      // entire initializer has been compiled. This prevents ambiguous use of the
      // same variable name in the initializer as the one being declared. E.g.:
      //  var x: 1
      //  block
      //    var x: x + 1
      //  end
      if (!is_initialized(existing_variable))
        error(parser, "You cannot use the variable's name being declared in its initializer.");

      // The order and position of the `variables` array will be
      // identical to how they end up on the stack.
      return i;
    }
  }

  return NOT_FOUND;
}

/// Check wether a given token is an assignment operator.
static bool is_assignment_operator(Token* token) {
  switch (token->type) {
    case TOKEN_COLON:
    case TOKEN_PLUS_COLON:
    case TOKEN_MINUS_COLON:
    case TOKEN_STAR_COLON:
    case TOKEN_SLASH_COLON:
      return true;
    default:
      return false;
  }
}

/// Write an instruction to assign a value to the variable at the given stack slot.
/// (Callers should have verified that the current token is an assignment operator.)
static void assign_variable(Parser* parser, byte stack_slot) {
  advance(parser);
  Token operator = parser->previous;
  TokenType type = operator.type;
  if (type == TOKEN_COLON) {
    parse_expression(parser);
  }
  // Augmented assignments.
  else {
    // Add the variable's value to the stack first to ensure correct order of operation.
    write_instructions(parser, OP_GET_VAR, stack_slot);
    parse_expression(parser);

    if (type == TOKEN_PLUS_COLON)
      write_instruction(parser, OP_ADD);
    else if (type == TOKEN_MINUS_COLON)
      write_instruction(parser, OP_SUBTRACT);
    else if (type == TOKEN_STAR_COLON)
      write_instruction(parser, OP_MULTIPLY);
    else if (type == TOKEN_SLASH_COLON)
      write_instruction(parser, OP_DIVIDE);
    else
      error_at(parser, &operator, "Internal error. Expected an assignment operator.");
  }
  write_instructions(parser, OP_SET_VAR, stack_slot);
}

/// Write an instruction to assign a value to the variable name provided if it
/// is assignable. Otherwise, write an instruction to access/load a variable.
static void access_or_assign_variable(Parser* parser, Token name, bool is_assignable) {
  int stack_slot = resolve(parser, &name);
  if (stack_slot == NOT_FOUND)
    error_at(parser, &name, "The variable has not been declared. Use 'var <name> : <value>' to declare it first.");

  if (is_assignable)
    assign_variable(parser, (byte)stack_slot);
  else
    write_instructions(parser, OP_GET_VAR, (byte)stack_slot);
}

// ---------------------------------------------------
// STATEMENTS
// ---------------------------------------------------

static void parse_statement(Parser* parser) {
  if (match(parser, TOKEN_VAR))
    parse_var_statement(parser);
  else if (match(parser, TOKEN_OUT))
    parse_out_statement(parser);
  else if (match(parser, TOKEN_IF))
    parse_if_statement(parser);
  else if (match(parser, TOKEN_BLOCK))
    parse_block_statement(parser);
  else if (match(parser, TOKEN_FOREACH))
    parse_foreach_statement(parser);
  else if (match(parser, TOKEN_WHILE))
    parse_while_statement(parser);
  else
    parse_expression_statement(parser);

  if (parser->panic_mode)
    synchronize(parser);
}

/// A standard block includes the full block from NEWLINE to 'end' and NEWLINE.
/// Use `_without_scope()` if the caller has already created a scope, e.g. for
/// functions and `foreach` loops to account for arguments and loop variables.
static void parse_standard_block_without_scope(Parser* parser) {
  consume_newline(parser);
  while (!is_at_end_of_block(parser) && !is_at_end_of_file(parser))
    parse_statement(parser);

  consume_end_of_block(parser);
}

/// A standard block includes the full block from NEWLINE to 'end' and NEWLINE.
/// Use `_with_scope()` if the caller has not already created a scope, e.g. for
/// standalone `block` statements or `while` loops.
static void parse_standard_block_with_scope(Parser* parser) {
  create_scope(parser);
  parse_standard_block_without_scope(parser);
  discard_scope(parser);
}

/// A selection block (used in `if`, `elseif`, `else`) does not include the 'end' keyword in
/// the grammar. Instead, 'end' and NEWLINE are assumed to be consumed correctly by the caller.
static void parse_selection_block(Parser* parser) {
  create_scope(parser);
  consume_newline(parser);
  // TODO: Also check against TOKEN_ELSEIF when it's added.
  while (!compare(parser, TOKEN_ELSE) && !is_at_end_of_block(parser) && !is_at_end_of_file(parser))
    parse_statement(parser);

  discard_scope(parser);
}

static void parse_block_statement(Parser* parser) {
  parse_standard_block_with_scope(parser);
}

static void parse_expression_statement(Parser* parser) {
  parse_expression(parser);
  consume_newline(parser);
  write_instruction(parser, OP_POP);
}

/// Grammar: `"foreach" IDENTIFIER "in" expression ".." expression ( "step" expression )? standardBlock`
static void parse_foreach_statement(Parser* parser) {
  // Create a scope in order to scope the loop variable.
  create_scope(parser);

  // --- Implicit Declaration: ---
  consume(parser, TOKEN_IDENTIFIER, "A name for the variable in the loop is missing. Add a name between 'foreach' and 'in'.");
  declare_variable(parser);
  Token loop_variable_name = parser->previous;

  // --- Initialization: ---
  consume(parser, TOKEN_IN, "You must use the 'in' keyword after the variable name.");
  parse_expression(parser);
  // The variable must be defined After the initializer has been parsed in order to prevent
  // use of it in the implicit initializer (left-hand side of `..`).
  define_variable(parser);
  byte loop_variable_slot = (byte)resolve(parser, &loop_variable_name);
  consume(parser, TOKEN_DOT_DOT, "You must use '..' with two surrounding expressions for the loop range. (E.g. '0..3')");

  // --- Condition: ---
  int condition_start_offset = get_current_instruction_offset(parser);
  // Parse the right-hand side (rhs) of `..` and compare it against the loop variable (i.e. `variable <= rhs`).
  write_instructions(parser, OP_GET_VAR, loop_variable_slot);
  parse_expression(parser);
  write_instruction(parser, OP_LESS_THAN_EQUALS);
  // Always jump over the `step` part.
  int placeholder_jump_to_body = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_TRUE);
  int placeholder_jump_to_end = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_FALSE);

  // --- Step: ---
  int step_start_offset = get_current_instruction_offset(parser);
  // Get the loop variable before the `step` to enforce the addition order `variable + step`.
  write_instructions(parser, OP_GET_VAR, loop_variable_slot);
  if (match(parser, TOKEN_STEP))
    parse_expression(parser);
  else
    // If `step` is omitted, there is an implicit `step` of 1.
    write_constant_instruction(parser, FROM_C_DOUBLE(1));

  // Add the two previous values and assign it to the loop variable (i.e. `variable: variable + step`).
  write_instruction(parser, OP_ADD);
  write_instructions(parser, OP_SET_VAR, loop_variable_slot);
  // Pop the assignment value.
  write_instruction(parser, OP_POP);
  write_jump_backward_instruction(parser, condition_start_offset);

  // --- Body: ---
  patch_jump_forward_instruction(parser, placeholder_jump_to_body);
  // Pop the condition value.
  write_instruction(parser, OP_POP);
  parse_standard_block_without_scope(parser);
  write_jump_backward_instruction(parser, step_start_offset);

  // --- End: ---
  patch_jump_forward_instruction(parser, placeholder_jump_to_end);
  // Pop the condition value.
  write_instruction(parser, OP_POP);
  discard_scope(parser);
}

static void parse_if_statement(Parser* parser) {
  // --- If-Condition: ---
  parse_expression(parser);
  int placeholder_jump_over_if = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_FALSE);

  // --- If-Then Body: ---
  write_instruction(parser, OP_POP);
  parse_selection_block(parser);
  int placeholder_jump_over_else = write_jump_forward_instruction(parser, OP_JUMP_FWD);

  // --- Else-Then Body: ---
  patch_jump_forward_instruction(parser, placeholder_jump_over_if);
  // Pop the if-condition value.
  write_instruction(parser, OP_POP);
  if (match(parser, TOKEN_ELSE))
    parse_selection_block(parser);
  consume_end_of_block(parser);

  // --- End: ---
  patch_jump_forward_instruction(parser, placeholder_jump_over_else);
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

/// Grammar: `"while" expression ( "{" expression "}" )? standardBlock`
static void parse_while_statement(Parser* parser) {
  // --- Condition: ---
  int condition_start_offset = get_current_instruction_offset(parser);
  parse_expression(parser);
  // Always jump over the modification part.
  int placeholder_jump_to_body = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_TRUE);
  int placeholder_jump_to_end = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_FALSE);

  // --- Optional Modification: ---
  int modification_start_offset = get_current_instruction_offset(parser);
  bool has_modification_expr = match(parser, TOKEN_OPEN_BRACE);
  if (has_modification_expr) {
    parse_expression(parser);
    // Pop the modification expression value.
    write_instruction(parser, OP_POP);
    consume(parser, TOKEN_CLOSE_BRACE, "The expression must be enclosed in `{ }`. Add `}` to terminate it.");
    write_jump_backward_instruction(parser, condition_start_offset);
  }

  // --- Body: ---
  patch_jump_forward_instruction(parser, placeholder_jump_to_body);
  // Pop the condition value.
  write_instruction(parser, OP_POP);
  parse_standard_block_with_scope(parser);
  write_jump_backward_instruction(parser, has_modification_expr ? modification_start_offset : condition_start_offset);

  // --- End: ---
  patch_jump_forward_instruction(parser, placeholder_jump_to_end);
  // Pop the condition value.
  write_instruction(parser, OP_POP);
}

// ---------------------------------------------------
// EXPRESSIONS
// ---------------------------------------------------

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
  bool is_assignable = is_assignment_operator(&parser->current) && min_precedence <= PRECEDENCE_ASSIGNMENT;
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
  if (!is_assignable && is_assignment_operator(&parser->current)) {
    advance(parser);
    error(parser, "You are trying to assign a value to an invalid target.");
  }
}

static void parse_expression(Parser* parser) {
  // Lowest precedence = PRECEDENCE_ASSIGNMENT
  parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
}

static void parse_and(Parser* parser, bool _) {
  // Jump to the end if the left condition is false.
  int placeholder_jump_over_and = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_FALSE);

  // Pop the left condition and continue parsing the right-hand side.
  write_instruction(parser, OP_POP);
  parse_precedence(parser, PRECEDENCE_CONJUNCTION);

  // Jump lands here if the left condition is false.
  patch_jump_forward_instruction(parser, placeholder_jump_over_and);

  // The last value left on the stack is the result of the expression and
  // should therefore not be popped.
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

static void parse_or(Parser* parser, bool _) {
  // Jump to the end if the left condition is true.
  int placeholder_jump_over_or = write_jump_forward_instruction(parser, OP_JUMP_FWD_IF_TRUE);

  // Pop the left condition and continue parsing the right-hand side.
  write_instruction(parser, OP_POP);
  parse_precedence(parser, PRECEDENCE_DISJUNCTION);

  // Jump lands here if the left condition is true.
  patch_jump_forward_instruction(parser, placeholder_jump_over_or);

  // The last value left on the stack is the result of the expression and
  // should therefore not be popped.
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

  #ifdef DEBUG_MODE
    if (flag_debug_compilation && !parser->saw_error)
      disassemble_program(get_writable_program(parser));
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

  while (!match(&parser, TOKEN_EOF)) {
    parse_statement(&parser);
  }

  end_compilation(&parser);

  return !parser.saw_error;
}
