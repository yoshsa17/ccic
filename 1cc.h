#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//// tokenize

typedef enum {
  TOKEN_RESERVED, // symbol (e.g "+", "-")
  TOKEN_IDENT,
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

extern Token *token;
extern char *user_input;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token* consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenType type, Token *current, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();

//// codegen

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
  ND_ASSIGN, // =
  ND_LVAR,   // local variable
} NodeType;

typedef struct Node Node;
struct Node {
  NodeType type; 
  Node *lhs;     // left child node
  Node *rhs;     // right child node
  int val;       // only type == ND_NUM
  int offset;  // only type == ND_LVAR
};

extern Node *code[];

Node *new_node(NodeType type); 
Node *new_binary(NodeType type, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Node *assign();
Node *stmt();
void program();

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen_lval(Node *node);
void gen(Node *node);