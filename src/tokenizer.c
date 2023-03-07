#include "tokenizer.h"

void init_tokenizer(Tokenizer* tokenizer, const char* source) {
  tokenizer->start = source;
  tokenizer->current = source;
  tokenizer->line = 1;
}

Token tokenize(Tokenizer* tokenizer) {
  // TODO
}
