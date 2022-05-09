#include "1cc.h"

// token currently focusing on
Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%p\n", loc);
  fprintf(stderr, "%p\n", user_input);
  fprintf(stderr, "%d\n", pos);
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // print whitespaces
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// If the next token is the expected symbol, proceed one token forward 
// and return true. Otherwise return false.
bool consume(char *op) {
  if (token->type != TOKEN_RESERVED ||
      strlen(op) != token->len ||
      // memcmp => return 0 if equal => if(false)
      memcmp(token->str, op, token->len)) {

    return false;
  }
  token = token->next;
  return true;
}

// If the next token is the expected symbol, proceed one token forward.
// Otherwise, throw an error.
void expect(char *op) {
  if (token->type != TOKEN_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {

    error_at(token->str, "token is not \"%s\"", op);
  }
  token = token->next;
}

// If the next token is a number, proceed one token forward 
// and return that number. Otherwise, throw an error.
int expect_number() {
  if (token->type != TOKEN_NUM) {
    error_at(token->str, "token is not a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->type == TOKEN_EOF;
}

Token *new_token(TokenType type, Token *current, char *str, int len) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->str = str;
  token->len = len;
  current->next = token;
  return token;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize() {
  char *p = user_input;

  // create a fake head element and return head->next at the end
  Token head;
  head.next = NULL;
  Token *current = &head;

  while (*p) {
    // skip white-space characters
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") ||
        startswith(p, "!=") ||
        startswith(p, "<=") ||
        startswith(p, ">=")) {
      current = new_token(TOKEN_RESERVED, current, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
      current = new_token(TOKEN_RESERVED, current, p, 1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      current = new_token(TOKEN_NUM, current, p, 0);
      char *q = p;
      current->val = strtol(p, &p, 10);
      current->len = p - q;
      continue;
    }

    error_at(p, "Unable to tokenize");
  }

  // now *p == '\0'
  new_token(TOKEN_EOF, current, p, 0);
  return head.next;
}