#include <string.h>

#include "tokenizer.h"

void tokenizer_init(Tokenizer* tokenizer, const char* source) {
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
  token.type = TOKEN_LEXICAL_ERROR;
  token.lexeme = message;
  token.length = (int)(strlen(message));
  token.line = tokenizer->line;

  return token;
}

static bool is_alpha(char character) {
  return (character >= 'a' && character <= 'z')
    || (character >= 'A' && character <= 'Z')
    || character == '_';
}

static bool is_digit(char character) {
  return character >= '0' && character <= '9';
}

static bool is_alphanumeric(char character) {
  return is_alpha(character) || is_digit(character);
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

static inline bool is_whitespace(Tokenizer* tokenizer) {
  // TODO: Double check '\r' and '\r\n'.
  char character = peek(tokenizer);
  return character == ' ' || character == '\t' || character == '\r' || (tokenizer->is_blank_line && character == '\n');
}

static void skip_whitespace(Tokenizer* tokenizer) {
  while (is_whitespace(tokenizer)) {
    if (peek(tokenizer) == '\n')
      tokenizer->line++;

    advance(tokenizer);
  }
}

static bool is_comment(Tokenizer* tokenizer) {
  return peek(tokenizer) == '/' && peek_next(tokenizer) == '/';
}

static void skip_comment(Tokenizer* tokenizer) {
  char character = peek(tokenizer);
  if (is_comment(tokenizer)) {
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

static void skip_insignificant(Tokenizer* tokenizer) {
  while (is_whitespace(tokenizer) || is_comment(tokenizer)) {
    skip_whitespace(tokenizer);
    skip_comment(tokenizer);
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

static TokenType search_keyword(Tokenizer* tokenizer, int prefix_length, const char* suffix, int suffix_length, TokenType type) {
  int lexeme_length = tokenizer->current - tokenizer->start;
  const char* lexeme_suffix = tokenizer->start + prefix_length;
  bool is_exact_match =
    lexeme_length == prefix_length + suffix_length
    && memcmp(lexeme_suffix, suffix, suffix_length) == 0;

  return is_exact_match ? type : TOKEN_IDENTIFIER;
}

static TokenType get_keyword_or_identifier_type(Tokenizer* tokenizer) {
  // Using a trie to check if it is one of the keywords.
  int lexeme_length = tokenizer->current - tokenizer->start;
  switch (tokenizer->start[0]) {
    case '@':
      return search_keyword(tokenizer, 1, "out", 3, TOKEN_OUT);
    case 'a':
      return search_keyword(tokenizer, 1, "nd", 2, TOKEN_AND);
    case 'b':
      return search_keyword(tokenizer, 1, "lock", 4, TOKEN_BLOCK);
    case 'e':
      if (lexeme_length > 1) {
        switch (tokenizer->start[1]) {
          case 'l':
            return search_keyword(tokenizer, 2, "se", 2, TOKEN_ELSE);
          case 'n':
            return search_keyword(tokenizer, 2, "d", 1, TOKEN_END);
        }
      }
      break;
    case 'f':
      if (lexeme_length > 1) {
        switch (tokenizer->start[1]) {
          case 'a':
            return search_keyword(tokenizer, 2, "lse", 3, TOKEN_FALSE);
          case 'o':
            return search_keyword(tokenizer, 2, "reach", 5, TOKEN_FOREACH);
        }
      }
      break;
    case 'i':
      if (lexeme_length > 1) {
        switch (tokenizer->start[1]) {
          case 'f':
            return search_keyword(tokenizer, 2, "", 0, TOKEN_IF);
          case 'n':
            return search_keyword(tokenizer, 2, "", 0, TOKEN_IN);
        }
      }
      break;
    case 'm':
      return search_keyword(tokenizer, 1, "od", 2, TOKEN_MOD);
    case 'n':
      if (lexeme_length > 2) {
        switch (tokenizer->start[1]) {
          case 'o':
            switch (tokenizer->start[2]) {
              case 'n':
                return search_keyword(tokenizer, 3, "e", 1, TOKEN_NONE);
              case 't':
                return search_keyword(tokenizer, 3, "", 0, TOKEN_NOT);
            }
            break;
        }
      }
      break;
    case 'o':
      return search_keyword(tokenizer, 1, "r", 1, TOKEN_OR);
    case 's':
      return search_keyword(tokenizer, 1, "tep", 3, TOKEN_STEP);
    case 't':
      return search_keyword(tokenizer, 1, "rue", 3, TOKEN_TRUE);
    case 'v':
      return search_keyword(tokenizer, 1, "ar", 2, TOKEN_VAR);
    case 'w':
      return search_keyword(tokenizer, 1, "hile", 4, TOKEN_WHILE);
  }

  return TOKEN_IDENTIFIER;
}

static Token consume_keyword_or_identifier(Tokenizer* tokenizer) {
  while (is_alphanumeric(peek(tokenizer)))
    advance(tokenizer);

  return make_token(tokenizer, get_keyword_or_identifier_type(tokenizer));
}

Token tokenize(Tokenizer* tokenizer) {
  skip_insignificant(tokenizer);

  tokenizer->is_blank_line = false;
  tokenizer->start = tokenizer->current;

  if (is_at_end(tokenizer))
    return make_token(tokenizer, TOKEN_EOF);

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
      return match(tokenizer, ':')
        ? make_token(tokenizer, TOKEN_PLUS_COLON)
        : make_token(tokenizer, TOKEN_PLUS);
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
      return make_error_token(tokenizer, "You have included an illegal character: ! (This character is only allowed in `!=`. Did you mean `not`?)");
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
    case '@': {
      Token token = consume_keyword_or_identifier(tokenizer);
      // Temporary until built-in functions are supported.
      return token.type == TOKEN_OUT
        ? token
        : make_error_token(tokenizer, "'@' is only allowed in names of the built-in functionality.");
    }
    // TODO: Move this condition earlier once TOKEN_DOT is added.
    case '.': {
      if (match(tokenizer, '.'))
        return make_token(tokenizer, TOKEN_DOT_DOT);
      return make_error_token(tokenizer, "You have included an illegal character: . (This character is only allowed as `..`.)");
    }
    case '{':
      return make_token(tokenizer, TOKEN_OPEN_BRACE);
    case '}':
      return make_token(tokenizer, TOKEN_CLOSE_BRACE);
    default:
      // TODO: Consider putting these first (before the switch).
      if (is_alpha(character))
        return consume_keyword_or_identifier(tokenizer);
      if (is_digit(character))
        return consume_number(tokenizer);

      return make_error_token(tokenizer, "You have included an illegal character.");
  }
}
