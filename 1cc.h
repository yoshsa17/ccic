#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//// Tokenize

typedef enum {
  TOKEN_RESERVED, // symbol (e.g "+", "-")
  TOKEN_IDENT,
  TOKEN_RETURN,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,
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

typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset; 
};

extern Token *token;
extern char *user_input;
extern LVar *locals;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_type(TokenType type);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenType type, Token *current, char *str, int len);
bool startswith(char *p, char *q);
LVar *find_lvar(Token *tok);
int is_alnum(char c);
Token *tokenize();

//// Parse

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
  ND_RETURN, // return
  ND_IF, // if
  ND_ELSE, // else
  ND_WHILE, // while
  ND_FOR,  // for
  ND_FOR_LEFT,
  ND_FOR_RIGHT,
} NodeType;

typedef struct Node Node;
struct Node {
  NodeType type; 
  Node *lhs;     // left child node
  Node *rhs;     // right child node
  Node *els;     // only type == ND_IF
  int val;       // only type == ND_NUM
  int offset;  // only type == ND_LVAR
};

extern Node *code[];

Node *new_node(NodeType type); 
Node *new_binary(NodeType type, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void program();
Node *stmt();
Node *expr();
Node *assign();

Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

//// Codegen

void gen_lval(Node *node);
void gen(Node *node);