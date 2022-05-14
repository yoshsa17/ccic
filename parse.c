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

Node *code[100];

// program    = stmt*
void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

// stmt       = expr ";" 
//          | "return" expr ";"
//          | "while" "(" expr ")" stmt
//          | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//          | ...
Node *stmt() {
  Node *node;

  if(consume_type(TOKEN_FOR)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->type = ND_FOR;

    Node *left = calloc(1, sizeof(Node));
    left->type = ND_FOR_LEFT;
    Node *right = calloc(1, sizeof(Node));
    right->type = ND_FOR_RIGHT;

    if(!consume(";")) {
      left->lhs = expr();
      expect(";");
    }

    if(!consume(";")) {
      left->rhs = expr();
      expect(";");
    }
    
    if(!consume(")")) {
      right->lhs = expr();
      expect(")");
    }

    right->rhs = stmt();

    node->lhs = left;
    node->rhs = right;

    return node;
  }


  if(consume_type(TOKEN_WHILE)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->type = ND_WHILE;
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  }

  if(consume_type(TOKEN_IF)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->type = ND_IF;
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();

    if(consume_type(TOKEN_ELSE)) {
      Node *els = calloc(1, sizeof(Node));
      els->type = ND_ELSE;
      els->lhs = node->rhs;
      els->rhs = stmt();
      node->rhs = els;
    } 

    return node;
  }

  if(consume_type(TOKEN_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->type = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }
  expect(";");
  return node;
}

// expr       = assign
Node *expr() {
  return assign();
}

// assign     = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
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

// primary    = num | ident | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_type(TOKEN_IDENT);
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if(locals == NULL) {
        lvar->offset = 8;
      } else  {
        lvar->offset = locals->offset + 8;
      }
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }
  return new_node_num(expect_number());
}
