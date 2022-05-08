#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_RESERVED, // symbol (e.g "+", "-")
  TOKEN_NUM,      // number
  TOKEN_EOF,      // token representing the end of the input
} TokenType;

typedef struct Token Token;
struct Token {
  TokenType type; 
  Token *next; 
  int val;
  char *str;
};

// token currently focusing on
Token *token;

// 
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// If the next token is the expected symbol, proceed one token forward 
// and return true. Otherwise return false.
bool consume(char op) {
  if (token->type != TOKEN_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

// If the next token is the expected symbol, proceed one token forward.
// Otherwise, throw an error.
void expect(char op) {
  if (token->type != TOKEN_RESERVED || token->str[0] != op) {
    error("token is not '%c'", op);
  }
  token = token->next;
}

// If the next token is a number, proceed one token forward 
// and return that number. Otherwise, throw an error.
int expect_number() {
  if (token->type != TOKEN_NUM) {
    error("token is not a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->type == TOKEN_EOF;
}

Token *new_token(TokenType type, Token *current, char *str) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->str = str;
  current->next = token;
  return token;
}

Token *tokenize(char *p) {
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

    if (*p == '+' || *p == '-') {
      current = new_token(TOKEN_RESERVED, current, p);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      current = new_token(TOKEN_NUM, current, p);
      current->val = strtol(p, &p, 10);
      continue;
    }

    error("Unable to tokenize");
  }

  // now *p == '\0'
  new_token(TOKEN_EOF, current, p);
  return head.next;
}

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "invalid args\n");
    return 1;
  }

  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}