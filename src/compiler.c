#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "tokenizer.h"

void compile(VM* vm, const char* source) {
  Tokenizer tokenizer;
  init_tokenizer(&tokenizer, source);

  int line = -1;
  while (true) {
    Token token = tokenize(&tokenizer);
    bool same_line_as_previous = token.line == line;
    if (same_line_as_previous)
      printf("           ");
    else {
      printf("line[%4d] ", token.line);
      line = token.line;
    }
    // `token.length` is passed as the argument for '*' (determining how many
    // characters of `token.start` to display). E.g. tokentype[15] lexeme[else]'.
    printf("tokentype[%2d] lexeme[%.*s]\n", token.type, token.length, token.lexeme); 

    if (token.type == TOKEN_FILE_END)
      break;
  }
}
