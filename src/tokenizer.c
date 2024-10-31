#include <string.h>

#include "tokenizer.h"

void tokenizer_init(Tokenizer* tokenizer, const char* source) {
  tokenizer->start = source;
  tokenizer->current = source;
  tokenizer->line = 1;
  tokenizer->is_blank_line = true;
}

/// Make a token from the current lexeme scanned.
static Token make_token(Tokenizer* tokenizer, TokenType type) {
  Token token;
  token.type = type;
  token.lexeme = tokenizer->start;
  token.length = (int)(tokenizer->current - tokenizer->start);
  token.line = tokenizer->line;

  return token;
}

/// Make a sentinel error token, storing the error message as the lexeme.
static Token make_error_token(Tokenizer* tokenizer, const char* message) {
  Token token;
  token.type = TOKEN_LEXICAL_ERROR;
  token.lexeme = message;
  token.length = (int)(strlen(message));
  token.line = tokenizer->line;

  return token;
}

/// Check whether a character is an alpha (a-z, A-Z) or underscore.
static bool is_alpha(char character) {
  return (character >= 'a' && character <= 'z')
    || (character >= 'A' && character <= 'Z')
    || character == '_';
}

/// Check whether a character is a digit (0-9).
static bool is_digit(char character) {
  return character >= '0' && character <= '9';
}

/// Check whether a character is alphanumeric (a-z, A-Z, 0-9) or underscore.
static bool is_alphanumeric(char character) {
  return is_alpha(character) || is_digit(character);
}

/// Check whether the tokenizer has reached the end of the source string.
static bool is_at_end(Tokenizer* tokenizer) {
  return *tokenizer->current == '\0';
}

/// Advance to the next character and return the just consumed one.
static char advance(Tokenizer* tokenizer) {
  tokenizer->current++;

  return tokenizer->current[-1];
}

/// Look ahead at the current character to be consumed without consuming it.
static char peek(Tokenizer* tokenizer) {
  return *tokenizer->current;
}

/// Look ahead at the second character to be consumed without consuming it.
static char peek_next(Tokenizer* tokenizer) {
  if (is_at_end(tokenizer))
    return '\0';

  return tokenizer->current[1];
}

/// Check whether the current unconsumed character matches an expected
/// character, and consume it if they match.
static bool match(Tokenizer* tokenizer, char expected) {
  if (is_at_end(tokenizer) || *tokenizer->current != expected)
    return false;

  advance(tokenizer);

  return true;
}

/// Check whether the current unconsumed character should be treated as whitespace.
/// This includes newline characters on blank lines.
static inline bool is_whitespace(Tokenizer* tokenizer) {
  // TODO: Double check '\r' and '\r\n'.
  char character = peek(tokenizer);
  return character == ' ' || character == '\t' || character == '\r' || (tokenizer->is_blank_line && character == '\n');
}

/// Advance the tokenizer passed all consecutively encountered whitespace.
static void skip_whitespace(Tokenizer* tokenizer) {
  while (is_whitespace(tokenizer)) {
    if (peek(tokenizer) == '\n')
      tokenizer->line++;

    advance(tokenizer);
  }
}

/// Check whether the next unconsumed characters indicate a comment.
static bool is_comment(Tokenizer* tokenizer) {
  return peek(tokenizer) == '/' && peek_next(tokenizer) == '/';
}

/// Advance the tokenizer to the end of the line.
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

/// Advance the tokenizer passed all consecutive whitespace and comments.
static void skip_insignificant(Tokenizer* tokenizer) {
  while (is_whitespace(tokenizer) || is_comment(tokenizer)) {
    skip_whitespace(tokenizer);
    skip_comment(tokenizer);
  }
}

/// Consume the current number literal.
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

/// Consume the current text literal.
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

/// Search for a keyword where its suffix matches the provided suffix,
/// and return the corresponding keyword token type if they match,
/// otherwise return an identifier token type.
///
/// By the time this function is called, it is known that the prefix of the
/// current lexeme being scanned matches the prefix of the keyword represented
/// by the token type provided. Thus, only the keyword suffix and the
/// provided suffix are compared to see if the current lexeme is a keyword.
static TokenType search_keyword(Tokenizer* tokenizer, int prefix_length, const char* suffix, int suffix_length, TokenType type) {
  int lexeme_length = tokenizer->current - tokenizer->start;
  const char* lexeme_suffix = tokenizer->start + prefix_length;
  bool is_exact_match =
    lexeme_length == prefix_length + suffix_length
    && memcmp(lexeme_suffix, suffix, suffix_length) == 0;

  return is_exact_match ? type : TOKEN_IDENTIFIER;
}

/// Get a keyword token type if the current lexeme being scanned matches
/// a reserved keyword, otherwise get an identifier token type.
///
/// A trie is used for searching for a keyword.
static TokenType get_keyword_or_identifier_type(Tokenizer* tokenizer) {
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

/// Consume the current keyword or identifier.
static Token consume_keyword_or_identifier(Tokenizer* tokenizer) {
  while (is_alphanumeric(peek(tokenizer)))
    advance(tokenizer);

  return make_token(tokenizer, get_keyword_or_identifier_type(tokenizer));
}

/// Generate and return (by value) the next token found in the source code.
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
      return match(tokenizer, ':')
        ? make_token(tokenizer, TOKEN_MINUS_COLON)
        : make_token(tokenizer, TOKEN_MINUS);
    case '*':
      return match(tokenizer, ':')
        ? make_token(tokenizer, TOKEN_STAR_COLON)
        : make_token(tokenizer, TOKEN_STAR);
    case '/':
      return match(tokenizer, ':')
        ? make_token(tokenizer, TOKEN_SLASH_COLON)
        : make_token(tokenizer, TOKEN_SLASH);
    case '=':
      return make_token(tokenizer, TOKEN_EQUALS);
    case '!':
      return match(tokenizer, '=')
        ? make_token(tokenizer, TOKEN_EXCLAMATION_EQUALS)
        : make_error_token(tokenizer, "You have included an illegal character: ! (This character is only allowed in `!=`. Did you mean `not`?)");
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
