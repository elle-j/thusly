#include <string.h>

#include "common.h"
#include "tokenizer.h"

void init_tokenizer(Tokenizer* tokenizer, const char* source) {
  tokenizer->start = source;
  tokenizer->current = source;
  tokenizer->line = 1;
}

static bool is_at_end(Tokenizer* tokenizer) {
  return *tokenizer->current == '\0';
}

static char advance(Tokenizer* tokenizer) {
  tokenizer->current++;

  return tokenizer->current[-1];
}

static bool match(Tokenizer* tokenizer, char expected) {
  if (is_at_end(tokenizer) || *tokenizer->current != expected)
    return false;

  advance(tokenizer);

  return true;
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

Token tokenize(Tokenizer* tokenizer) {
  tokenizer->start = tokenizer->current;

  if (is_at_end(tokenizer))
    return make_token(tokenizer, TOKEN_FILE_END);
  
  char character = advance(tokenizer);
  switch (character) {
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
      return make_error_token(tokenizer, "You have included an illegal character: !");
    case '<':
      return match(tokenizer, '=')
        ? make_token(tokenizer, TOKEN_LESS_THAN_EQUALS)
        : make_token(tokenizer, TOKEN_LESS_THAN);
    case '>':
      return match(tokenizer, '=')
        ? make_token(tokenizer, TOKEN_GREATER_THAN_EQUALS)
        : make_token(tokenizer, TOKEN_GREATER_THAN);
  }
  
  return make_error_token(tokenizer, "You have included an illegal character");
}
