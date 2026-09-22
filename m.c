#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 256
#define SPACES 2

enum TokenType {
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_PLUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_MINUS,
  TOKEN_EQUAL,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_SEMICOLON,
  TOKEN_UNKNOWN,
  TOKEN_EOF
};

enum ASTNodeType {
  AST_NUMBER,
  AST_IDENTIFIER,
  AST_BINARY,
  AST_ASSIGNMENT
};

struct Token {
  enum TokenType type;
  char *start;
  int  length;
};

struct Parser {
  struct Token *tokens;
  int pos;
  int has_error;
};

struct ASTNode {
  enum ASTNodeType kind;
  int  number_value;
  char identifier_name[BUFFER_SIZE];
  enum TokenType operator_kind;
  struct ASTNode *left;
  struct ASTNode *right;
};

struct ASTNode * createNumberNode(int value);
struct ASTNode * createIdentifierNode(char *name);
struct ASTNode * createBinaryNode(enum TokenType type, struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * parse_expression(struct Parser *p);
struct ASTNode * parse_primary(struct Parser *p);
struct ASTNode * parse_term(struct Parser *p);

char *node_type_name(enum ASTNodeType kind) {
  switch (kind) {
    case AST_NUMBER:
      return "NUMBER";
    case AST_IDENTIFIER:
      return "IDENTIFIER";
    case AST_BINARY:
      return "BINARY";
    case AST_ASSIGNMENT:
      return "ASSIGNMENT";
  }

  return "AST_UNKNOWN";
}

char *token_type_name(enum TokenType type) {
  switch (type) {
    case TOKEN_NUMBER:
      return "NUMBER";
    case TOKEN_IDENTIFIER:
      return "IDENTIFIER";
    case TOKEN_PLUS:
      return "+";
    case TOKEN_STAR:
      return "*";
    case TOKEN_SLASH:
      return "/";
    case TOKEN_MINUS:
      return "-";
    case TOKEN_EQUAL:
      return "=";
    case TOKEN_LPAREN:
      return "(";
    case TOKEN_RPAREN:
      return ")";
    case TOKEN_SEMICOLON:
      return ";";
    case TOKEN_UNKNOWN:
      return "UNKNOWN";
    case TOKEN_EOF:
      return "EOF";
  }
  return "UNKNOWN";
}

// lexer
void add_token(struct Token tokens[], int *token_count, enum TokenType type, char *start, int length) {
  struct Token t;
  t.type = type;
  t.start = start;
  t.length = length;
  tokens[*token_count] = t;
  *token_count = *token_count + 1;
  return;
}

void print_token(struct Token t) {
  if (t.type == TOKEN_EOF) {
    printf("EOF\n");
  } else {
    printf("%s: %.*s\n", token_type_name(t.type), t.length, t.start);
  }
}

void print_ast_tree(struct ASTNode *root, int depth) {
  struct ASTNode * t = root;

  if (t == NULL) {
    return;
  }
  if (t->kind == AST_ASSIGNMENT) {
    printf("%*s %s\n", SPACES*depth, ".", node_type_name(t->kind));
  } else if (t->kind == AST_NUMBER) {
    printf("%*s %s %d\n", SPACES*depth, ".", node_type_name(t->kind), t->number_value);
  } else if (t->kind == AST_IDENTIFIER) {
    printf("%*s %s %s\n", SPACES*depth, ".", node_type_name(t->kind), t->identifier_name);
  } else if (t->kind == AST_BINARY) {
    printf("%*s %s %s\n", SPACES*depth, ".", node_type_name(t->kind), token_type_name(t->operator_kind));
  }
  print_ast_tree(t->left, depth+1);
  print_ast_tree(t->right, depth+1);
}

int is_alpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

int is_equal(char c) {
  return (c == '=');
}

int is_star(char c) {
  return (c == '*');
}

int is_slash(char c) {
  return (c == '/');
}

int is_semicolon(char c) {
  return (c == ';');
}

int is_plus(char c) {
  return (c == '+');
}

int is_minus(char c) {
  return (c == '-');
}

int is_lparen(char c) {
  return (c == '(');
}

int is_rparen(char c) {
  return (c == ')');
}

int is_space(char c) {
  return (c == ' ' || c == '\t' || c == '\n');
}

int is_digit(char c) {
  if ( c >= '0' && c <= '9' ) {
    return 1;
  }
  return 0;
}

int is_alnum(char c) {
  return (is_digit(c) || is_alpha(c));
}

int scan_identifier(char *code, int start) {
  int i=start;
  while(is_alnum(code[i])) {
    i++;
  }
  return i;
}

int scan_number(char *code, int start) {
  int i=start;

  while (is_digit(code[i])) {
    i++;
  }

  return i;
}

void skip_spaces(char *code, int *i) {
  while(is_space(code[*i])) {
    *i = *i + 1;
  }
}

void tokenize(char *code, struct Token tokens[], int *token_count) {
  int idx = 0;
  while(code[idx] != '\0') {
    if (is_space(code[idx])) {
      skip_spaces(code, &idx);
    } else if (is_digit(code[idx])) {
      int start = idx;
      idx = scan_number(code, idx);
      add_token(tokens, token_count, TOKEN_NUMBER, code + start, idx - start);
    } else if (is_plus(code[idx])) {
      add_token(tokens, token_count, TOKEN_PLUS, code + idx, 1);
      idx++;
    } else if (is_minus(code[idx])) {
      add_token(tokens, token_count, TOKEN_MINUS, code + idx, 1);
      idx++;
    } else if (is_semicolon(code[idx])) {
      add_token(tokens, token_count, TOKEN_SEMICOLON, code + idx, 1);
      idx++;
    } else if (is_star(code[idx])) {
      add_token(tokens, token_count, TOKEN_STAR, code + idx, 1);
      idx++;
    } else if (is_slash(code[idx])) {
      add_token(tokens, token_count, TOKEN_SLASH, code + idx, 1);
      idx++;
    } else if (is_equal(code[idx])) {
      add_token(tokens, token_count, TOKEN_EQUAL, code + idx, 1);
      idx++;
    } else if (is_lparen(code[idx])) {
      add_token(tokens, token_count, TOKEN_LPAREN, code + idx, 1);
      idx++;
    } else if (is_rparen(code[idx])) {
      add_token(tokens, token_count, TOKEN_RPAREN, code + idx, 1);
      idx++;
    } else if (is_alpha(code[idx])) {
      int start = idx;
      idx = scan_identifier(code, idx);
      add_token(tokens, token_count, TOKEN_IDENTIFIER, code + start, idx - start);
    } else {
      add_token(tokens, token_count, TOKEN_UNKNOWN, code + idx, 1);
      idx++;
    }
  }
  add_token(tokens, token_count, TOKEN_EOF, code + idx, 0);
}

// Parser
int is_current_token(struct Parser *p, enum TokenType type) {
  struct Token t = p->tokens[p->pos];
  if (t.type == type) {
    return 1;
  }
  return 0;
}

void advance_token(struct Parser *p) {
  p->pos = p->pos + 1;
  return;
}

struct Token current_token(struct Parser *p) {
  return p->tokens[p->pos];
}

void expect_token(struct Parser *p, enum TokenType type) {
  struct Token t = p->tokens[p->pos];
  if (t.type == type) {
    //printf("Found token: %s\n", token_type_name(type));
    advance_token(p);
  } else {
    p->has_error = 1;
    printf("Didn't find the expected token: %s\n", token_type_name(type));
  }
  return;
}

struct ASTNode * createNumberNode(int value) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for ASTNode\n");
  } else {
    node->kind = AST_NUMBER;
    node->number_value = value;
    node->left = NULL;
    node->right = NULL;
    node->identifier_name[0] = '\0';
    node->operator_kind = TOKEN_EOF;
  }
  return node;
}

struct ASTNode * createIdentifierNode(char *name) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for ASTNode\n");
  } else {
    node->kind = AST_IDENTIFIER;
    node->number_value = -99999;
    node->left = NULL;
    node->right = NULL;
    size_t len = strlen(name);
    int bytesToCopy = (len >= BUFFER_SIZE) ? BUFFER_SIZE-1 : len;
    strncpy(node->identifier_name, name, bytesToCopy);
    node->identifier_name[bytesToCopy] = '\0';
    node->operator_kind = TOKEN_EOF;
  }
  return node;
}

struct ASTNode * createBinaryNode(enum TokenType type, struct ASTNode * leftNode, struct ASTNode * rightNode) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for ASTNode\n");
  } else {
    node->kind = AST_BINARY;
    node->number_value = -99999;
    node->left = leftNode;
    node->right = rightNode;
    node->identifier_name[0] = '\0';
    node->operator_kind = type;
  }
  return node;
}

struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for ASTNode\n");
  } else {
    node->kind = AST_ASSIGNMENT;
    node->number_value = -9999;
    node->left = leftNode;
    node->right = rightNode;
    node->identifier_name[0] = '\0';
    node->operator_kind = TOKEN_EOF;
  }
  return node;
}

// grammers
// number
// identifier
// assignment = identifier = number

// parser function per grammer rule
void parse_lparen(struct Parser *p) {
  if (p->has_error) {
    return;
  }
  expect_token(p, TOKEN_LPAREN);
  return;
}

void parse_rparen(struct Parser *p) {
  if (p->has_error) {
    return;
  }
  expect_token(p, TOKEN_RPAREN);
  return;
}

void parse_number(struct Parser *p) {
  if (p->has_error) {
    return;
  }
  expect_token(p, TOKEN_NUMBER);
  return;
}

struct ASTNode * parse_identifier(struct Parser *p) {
  if (p->has_error) {
    printf("Expected an identifier\n");
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_IDENTIFIER))  {
    struct Token token = p->tokens[p->pos];
    int bytesToCopy = (token.length >= BUFFER_SIZE) ? BUFFER_SIZE-1 : token.length;
    char name[BUFFER_SIZE];
    strncpy(name, token.start, bytesToCopy);
    name[bytesToCopy] = '\0';
    advance_token(p);
    node = createIdentifierNode(name);
    if (node == NULL) {
      p->has_error = 1;
    } else {
      return node;
    }
  } else {
    p->has_error = 1;
  }

  printf("Expected an identifier\n");
  return NULL;
}

// primary -> NUMBER | IDENTIFIER | LPAREN expression RPAREN
struct ASTNode * parse_primary(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;

  if (is_current_token(p, TOKEN_NUMBER)) {
    struct Token token = p->tokens[p->pos];
    int bytesToCopy = (token.length >= BUFFER_SIZE) ? BUFFER_SIZE-1 : token.length;
    char value[BUFFER_SIZE];
    strncpy(value, token.start, bytesToCopy);
    value[bytesToCopy] = '\0';
    advance_token(p);
    node = createNumberNode(atoi(value));
    if (node == NULL) {
      p->has_error = 1;
    }
    return node;
  } else if (is_current_token(p, TOKEN_IDENTIFIER))  {
    node = parse_identifier(p);
    return node;
  } else if (is_current_token(p, TOKEN_LPAREN)) {
    parse_lparen(p);
    node = parse_expression(p);
    if (node == NULL) {
      p->has_error = 1;
    } else {
      parse_rparen(p);
    }
    return node;
  }

  p->has_error = 1;
  printf("Expected primary\n");

  return NULL;
}

// term -> primary ((STAR | SLASH) primary)*
struct ASTNode * parse_term(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;

  left = parse_primary(p);
  if (left == NULL) {
    p->has_error = 1;
    return NULL;
  }

  while (p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_STAR)) {
      expect_token(p, TOKEN_STAR);  // consume the token
    } else if (is_current_token(p, TOKEN_SLASH)) {
      expect_token(p, TOKEN_SLASH); // consume the token
    } else {
      break;
    }

    if ( p->has_error) {
      break;
    }

    struct ASTNode * right = parse_primary(p);
    if (right == NULL) {
      p->has_error = 1;
      break;
    }

    struct ASTNode * node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
    } else {
      left = node;
    }
  }

  if (p->has_error) {
    return NULL;
  }

  return left;
}

// expression -> term ((PLUS | MINUS) term)*
struct ASTNode * parse_expression(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;
  left = parse_term(p);
  while (p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_PLUS)) {
      expect_token(p, TOKEN_PLUS);  // consume the token
    } else if (is_current_token(p, TOKEN_MINUS)) {
      expect_token(p, TOKEN_MINUS); // consume the token
    } else {
      break;
    }

    if ( p->has_error) {
      break;
    }

    struct ASTNode * right = parse_term(p);
    if (right == NULL) {
      p->has_error = 1;
      break;
    }

    struct ASTNode * node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
    } else {
      left = node;
    }
  }

  if (p->has_error) {
    return NULL;
  }

  return left;
}

// assignment -> IDENTIFIER EQUAL expression SEMICOLON
// expression -> primary
struct ASTNode * parse_assignment(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = parse_identifier(p);
  if ( left == NULL ) {
    p->has_error = 1;
    return NULL;
  }

  expect_token(p, TOKEN_EQUAL);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * right = parse_expression(p);
  if ( right == NULL ) {
    p->has_error = 1;
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * node = createAssignmentNode(left, right);
  if (node == NULL) {
    p->has_error = 1;
  }

  return node;
}

int main(void) {
  struct Token tokens[100];
  int token_count = 0;

  char *code = "x=123+45*2;";
  code="x=1+2+3;";
  code="x=1-2;";
  code="x=1+2-3-1+4;";
  code="x=1-;";
  code="x=1+2*3;";
  code="x=1*2+3;";
  code="x=1+2*3-4;";
  code="x=1-2*;";
  code="x=(2*3;";
  code="x=(1+2)*3+(4*1);";
  code="x=(1+2)*3+(4*1);";
  code="x=(2+3)*4;";
  code="x=1+2+3;";
  code="x=2*3*4;";
  code="x=(a+b)*c;";
  code="x=8/2;";
  code="x=8/2*3;";
  code="x=8/(2*4);";
  code="x=a/b+c;";
  code="x=(1+2)*3/(4*1);";

  printf("Input: %s\n", code);
  // Lexer
  tokenize(code, tokens, &token_count);
  for (int i = 0; i < token_count; i++) {
    print_token(tokens[i]);
  }

  // Parser
  struct Parser p;
  p.tokens = tokens;
  p.pos = 0;
  p.has_error = 0;

  struct ASTNode * node = parse_assignment(&p);
  if (p.has_error) {
    printf("Parsing failed\n");
  } else {
    printf("Parsing successful\n");
  }
  printf("Stopped at: ");
  print_token(current_token(&p));
  printf("Print AST Tree:\n ");
  print_ast_tree(node, 0);

  return 0;
}
