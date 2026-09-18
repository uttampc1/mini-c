#include <stdio.h>
#include <string.h>
#define BUFFER_SIZE 256

enum TokenType {
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_PLUS,
  TOKEN_STAR,
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
  AST_BINARY
};

struct Token {
  enum TokenType type;
  char *start;
  int length;
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
void parse_expression(struct Parser *p);

char *token_type_name(enum TokenType type) {
  switch (type) {
    case TOKEN_NUMBER:
      return "NUMBER";
    case TOKEN_IDENTIFIER:
      return "IDENTIFIER";
    case TOKEN_PLUS:
      return "PLUS";
    case TOKEN_STAR:
      return "STAR";
    case TOKEN_MINUS:
      return "MINUS";
    case TOKEN_EQUAL:
      return "EQUAL";
    case TOKEN_LPAREN:
      return "LPAREN";
    case TOKEN_RPAREN:
      return "RPAREN";
    case TOKEN_SEMICOLON:
      return "SEMICOLON";
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

int is_alpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

int is_equal(char c) {
  return (c == '=');
}

int is_star(char c) {
  return (c == '*');
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

#if 0 //NOT USED, handled directly in the tokenizer
void scan_equal(char *code, int index) {
  //printf("EQUAL: %c\n", code[index]);
}

void scan_star(char *code, int index) {
  //printf("STAR: %c\n", code[index]);
}

void scan_semicolon(char *code, int index) {
  //printf("SEMICOLON: %c\n", code[index]);
}

void scan_plus(char *code, int index) {
  //printf("PLUS: %c\n", code[index]);
}
#endif

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

void parse_identifier(struct Parser *p) {
  if (p->has_error) {
    return;
  }
  expect_token(p, TOKEN_IDENTIFIER);
  return;
}

// primary -> NUMBER | IDENTIFIER | LPAREN expression RPAREN
void parse_primary(struct Parser *p) {
  if (p->has_error) {
    return;
  }

  if (is_current_token(p, TOKEN_NUMBER)) {
    parse_number(p);
  } else if (is_current_token(p, TOKEN_IDENTIFIER))  {
    parse_identifier(p);
  } else if (is_current_token(p, TOKEN_LPAREN)) {
    parse_lparen(p);
    parse_expression(p);
    parse_rparen(p);
  } else {
    p->has_error = 1;
    printf("Expected primary\n");
  }
  return;
}

// primary -> NUMBER | IDENTIFIER
void parse_primary0(struct Parser *p) {
  if (p->has_error) {
    return;
  }

  if (is_current_token(p, TOKEN_NUMBER)) {
    parse_number(p);
  } else if (is_current_token(p, TOKEN_IDENTIFIER))  {
    parse_identifier(p);
  } else {
    p->has_error = 1;
    printf("Expected primary\n");
  }
  return;
}

// term -> primary (STAR primary)*
void parse_term(struct Parser *p) {
  if (p->has_error) {
    return;
  }

  parse_primary(p);
  while (p->has_error == 0 && is_current_token(p, TOKEN_STAR)) {
    expect_token(p, TOKEN_STAR);
    parse_primary(p);
  }

  return;
}

// expression -> term ((PLUS | MINUS) term)*
void parse_expression(struct Parser *p) {
  if (p->has_error) {
    return;
  }

  parse_term(p);
  while (p->has_error == 0) {
    if (is_current_token(p, TOKEN_PLUS)) {
      expect_token(p, TOKEN_PLUS);
    } else if (is_current_token(p, TOKEN_MINUS)) {
      expect_token(p, TOKEN_MINUS);
    } else {
      break;
    }
    parse_term(p);
  }
  return;
}
// expression -> primary ((PLUS | MINUS) primary)*
// x=y;
// x=1;
// x=1+; // syntax error. return on has_error
// x=1+2;
// x=1-2;
// x=a+b;
void parse_expression2(struct Parser *p) {
  if (p->has_error) {
    return;
  }

  parse_primary(p);
  while (p->has_error == 0) {
    if (is_current_token(p, TOKEN_PLUS)) {
      expect_token(p, TOKEN_PLUS);
    } else if (is_current_token(p, TOKEN_MINUS)) {
      expect_token(p, TOKEN_MINUS);
    } else {
      break;
    }
    parse_primary(p);
  }
  return;
}

// expression -> primary (PLUS primary)*
// x=y;
// x=1;
// x=1+; // syntax error. return on has_error
// x=1+2;
// x=a+b;
// x=1+2+3;
// x=1+2+3+b;
void parse_expression1(struct Parser *p) {
  if (p->has_error) {
    return;
  }

  parse_primary(p);
  while (p->has_error == 0 && is_current_token(p, TOKEN_PLUS)) {
    expect_token(p, TOKEN_PLUS);
    parse_primary(p);
  }
  return;
}

// expression -> primary
void parse_expression0(struct Parser *p) {
  parse_primary(p);
  return;
}

// assignment -> IDENTIFIER EQUAL NUMBER SEMICOLON
void parse_assignment0(struct Parser *p) {
  parse_identifier(p);
  expect_token(p, TOKEN_EQUAL);
  parse_number(p);
  expect_token(p, TOKEN_SEMICOLON);
  return;
}

// assignment -> IDENTIFIER EQUAL primary SEMICOLON
// primary -> NUMBER | IDENTIFIER
void parse_assignment1(struct Parser *p) {
  parse_identifier(p);
  expect_token(p, TOKEN_EQUAL);
  parse_primary(p);
  expect_token(p, TOKEN_SEMICOLON);
  return;
}

// assignment -> IDENTIFIER EQUAL expression SEMICOLON
// expression -> primary
void parse_assignment(struct Parser *p) {
  if (p->has_error) {
    return;
  }
  parse_identifier(p);
  expect_token(p, TOKEN_EQUAL);
  parse_expression(p);
  if (p->has_error) {
    return;
  }
  expect_token(p, TOKEN_SEMICOLON);
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
  code="x=(1+2)*3+(4*1);";
  code="x=(2*3;";

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

  parse_assignment(&p);
  if (p.has_error) {
    printf("Parsing failed\n");
  } else {
    printf("Parsing successful\n");
  }
  printf("Stopped at: ");
  print_token(current_token(&p));
  /*
  expect_token(&p, TOKEN_IDENTIFIER);
  expect_token(&p, TOKEN_EQUAL);
  struct Token t = current_token(&p);
  */

  return 0;
}
