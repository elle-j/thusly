#include <string.h>

#include "common.h"
#include "tokenizer.h"

void init_tokenizer(Tokenizer* tokenizer, const char* source) {
  tokenizer->start = source;
  tokenizer->current = source;
  tokenizer->line = 1;
  tokenizer->is_blank_line = true;
}

static Token make_token(Tokenizer* tokenizer, TokenType type) {
  Token token;
  token.type = type;
  token.lexeme = tokenizer->start;
  token.length = (int)(tokenizer->current - tokenizer->start);
  token.line = tokenizer->line;

  return token;
}

static Token make_error_token(Tokenizer* tokenizer, const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.lexeme = message;
  token.length = (int)(strlen(message));
  token.line = tokenizer->line;

  return token;
}

static bool is_digit(char character) {
  return character >= '0' && character <= '9';
}

static bool is_at_end(Tokenizer* tokenizer) {
  return *tokenizer->current == '\0';
}

static char advance(Tokenizer* tokenizer) {
  tokenizer->current++;

  return tokenizer->current[-1];
}

static char peek(Tokenizer* tokenizer) {
  return *tokenizer->current;
}

static char peek_next(Tokenizer* tokenizer) {
  if (is_at_end(tokenizer))
    return '\0';

  return tokenizer->current[1];
}

static bool match(Tokenizer* tokenizer, char expected) {
  if (is_at_end(tokenizer) || *tokenizer->current != expected)
    return false;

  advance(tokenizer);

  return true;
}

static inline bool is_whitespace(char character, bool is_blank_line) {
  // TODO: Double check '\r' (and '\r\n'?)
  return character == ' ' || character == '\t' || character == '\r' || (is_blank_line && character == '\n');
}

static void skip_whitespace(Tokenizer* tokenizer) {
  while (is_whitespace(peek(tokenizer), tokenizer->is_blank_line)) {
    if (peek(tokenizer) == '\n')
      tokenizer->line++;

    advance(tokenizer);
  }
}

static void skip_comment(Tokenizer* tokenizer) {
  char character = peek(tokenizer);
  if (character == '/' && peek_next(tokenizer) == '/') {
    while (character != '\n' && !is_at_end(tokenizer)) {
      advance(tokenizer);
      character = peek(tokenizer);
    }

    if (character == '\n' && tokenizer->is_blank_line) {
      // If it's not a blank line, let the switch statement in
      // `tokenize()` handle the significant newline instead.
      tokenizer->line++;
      advance(tokenizer);
    }
  }
}

static Token consume_number(Tokenizer* tokenizer) {
  while (is_digit(peek(tokenizer)))
    advance(tokenizer);

  if (peek(tokenizer) == '.' && is_digit(peek_next(tokenizer))) {
    // Consume the decimal point.
    advance(tokenizer);

    while (is_digit(peek(tokenizer)))
      advance(tokenizer);
  }

  return make_token(tokenizer, TOKEN_NUMBER);
}

static Token consume_text(Tokenizer* tokenizer) {
  // Save separate `line` variable rather than incrementing `tokenizer->line`
  // directly so that tokens store the line number at the start of the text.
  int line = tokenizer->line;
  while (peek(tokenizer) != '"' && !is_at_end(tokenizer)) {
    if (peek(tokenizer) == '\n')
      line++;
    
    advance(tokenizer);
  }

  Token token;
  if (is_at_end(tokenizer))
    token = make_error_token(tokenizer, "The text is not terminated. Use \" to terminate it.");
  else {
    // Consume the terminating ".
    advance(tokenizer);
    token = make_token(tokenizer, TOKEN_TEXT);
  }

  tokenizer->line = line;

  return token;
}

Token tokenize(Tokenizer* tokenizer) {
  skip_whitespace(tokenizer);
  skip_comment(tokenizer);

  tokenizer->is_blank_line = false;
  tokenizer->start = tokenizer->current;

  if (is_at_end(tokenizer))
    return make_token(tokenizer, TOKEN_FILE_END);

  char character = advance(tokenizer);
  switch (character) {
    case '\n': {
      // Semantically insignificant newlines are handled in `skip_whitespace()`.
      // (Make the token before incrementing line number.)
      Token token = make_token(tokenizer, TOKEN_NEWLINE);
      tokenizer->line++;
      tokenizer->is_blank_line = true;
      return token;
    }
    case ':':
      return make_token(tokenizer, TOKEN_COLON);
    case '(':
      return make_token(tokenizer, TOKEN_OPEN_PAREN);
    case ')':
      return make_token(tokenizer, TOKEN_CLOSE_PAREN);
    case '+':
      return make_token(tokenizer, TOKEN_PLUS);
    case '-':
      return make_token(tokenizer, TOKEN_MINUS);
    case '*':
      return make_token(tokenizer, TOKEN_STAR);
    case '/':
      return make_token(tokenizer, TOKEN_SLASH);
    case '=':
      return make_token(tokenizer, TOKEN_EQUALS);
    case '!':
      if (match(tokenizer, '='))
        return make_token(tokenizer, TOKEN_EXCLAMATION_EQUALS);
      return make_error_token(tokenizer, "You have included an illegal character: ! (This character is only allowed in !=)");
    case '<':
      return match(tokenizer, '=')
        ? make_token(tokenizer, TOKEN_LESS_THAN_EQUALS)
        : make_token(tokenizer, TOKEN_LESS_THAN);
    case '>':
      return match(tokenizer, '=')
        ? make_token(tokenizer, TOKEN_GREATER_THAN_EQUALS)
        : make_token(tokenizer, TOKEN_GREATER_THAN);
    case '"':
      return consume_text(tokenizer);
    default:
      if (is_digit(character))
        return consume_number(tokenizer);

      return make_error_token(tokenizer, "You have included an illegal character.");
  }
}
