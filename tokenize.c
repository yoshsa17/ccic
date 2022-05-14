#include "1cc.h"

Token *token;
char *user_input;
LVar *locals;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
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

Token *consume_type(TokenType type) {
  if (token->type != type) {
    return NULL;
  }
    Token* tok = token;
    token = token->next;
    return tok;
}

// If the next token is the expected symbol, proceed one token forward.
// Otherwise, throw an error.
void expect(char *op) {
  if (token->type != TOKEN_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {

    error_at(token->str, "expected:\"%s\" actual\"%s\"", op, token->str);
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

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

typedef struct ReservedWord ReservedWord;
struct ReservedWord {
  char *word;
  TokenType type;
};

ReservedWord reservedWord[] = {
  {"return", TOKEN_RETURN},
  {"if", TOKEN_IF},
  {"else", TOKEN_ELSE},
  {"while", TOKEN_WHILE},
  {"for", TOKEN_FOR},
  {"", TOKEN_EOF}
};



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

    if (strchr("+-*/()<>=;", *p)) {
      current = new_token(TOKEN_RESERVED, current, p, 1);
      p++;
      continue;
    }

    bool found = false;
    for(int i = 0; reservedWord[i].type != TOKEN_EOF; i++) {
      char *w = reservedWord[i].word;
      int len = strlen(w);
      TokenType type = reservedWord[i].type;
      if(startswith(p,w) && !is_alnum(p[len])) {
        current = new_token(type, current, p, len);
        p += len;
        found = true;
        break;
      }
    }
    if(found) {
      continue;
    }
    
    if('a' <= *p && *p <= 'z') {
      char *c = p;
      while('a' <= *c && *c <= 'z') {
        c++;
      }
      int len = c - p;

      current = new_token(TOKEN_IDENT, current, p, len);
      p = c;
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
