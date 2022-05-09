#include "1cc.h"

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