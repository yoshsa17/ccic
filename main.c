#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/////////////////////////////////
//
// Tokenizer
// 
/////////////////////////////////

typedef enum {
  TOKEN_RESERVED, // symbol (e.g "+", "-")
  TOKEN_NUM,      // number
  TOKEN_EOF,      // token representing the end of the input
} TokenType;

typedef struct Token Token;
struct Token {
  TokenType type; 
  Token *next; 
  int val;  // if token is a number, this represents its value
  char *str;
  int len; // token length for "==", "<=" etc.
};

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


/////////////////////////////////
// 
// Parser
// 
/////////////////////////////////

// AST node type
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // integer
} NodeType;

typedef struct Node Node;

struct Node {
  NodeType type; 
  Node *lhs;     // left child node
  Node *rhs;     // right child node
  int val;       // when NodeType is ND_NUM, it represents its value
};


Node *new_node(NodeType type) {
  Node *node = calloc(1, sizeof(Node));
  node->type = type;
  return node;
}

Node *new_binary(NodeType type, Node *lhs, Node *rhs) {
  Node *node = new_node(type);
  node->type = type;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// number node has no child nodes
Node *new_node_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr() {
  return equality();
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_binary(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_binary(ND_LE, node, add());
    } else if (consume(">")) {
      // use ND_LT and flip right node and left node
      node = new_binary(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_binary(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_binary(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

// mul   = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary   = ("+" | "-")? unary | primary
Node *unary() {
  if (consume("+")){
    return unary();
  }
  if (consume("-")) {
    // -x => 0-x
    return new_binary(ND_SUB, new_node_num(0), unary());
  }
  return primary();
}

// primary = num | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}

/////////////////////////////////
// 
// Code generator 
// 
/////////////////////////////////

void gen(Node *node) {
  if (node->type == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->type) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    // rax:64bit => 128bit
    printf("  cqo\n");
    // idiv rdi = rax+rdx / rdi) => rax(quotient), rdx(remainder) 
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    // store the result in the flags register
    printf("  cmp rax, rdi\n");
    // copy the value of the flags register in al
    printf("  sete al\n");
    // clear the remaining bits
    // rax(63bit) = | remaining(56bit) + al(8bit) |
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    // flip the value in the flags register and store it in al
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

/////////////////////////////////
//
// Main
//
/////////////////////////////////

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "invalid args\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}