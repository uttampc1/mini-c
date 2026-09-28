#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 256
#define MAX_SYMBOLS 100
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
  AST_ASSIGNMENT,
  AST_UNARY,
  AST_PROGRAM
};

enum TypeKind {
  TYPE_INT
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
  struct ASTNode * statements[BUFFER_SIZE];
  int    statement_count;
};

struct Symbol {
  enum TypeKind type;
  char *name;
  int value;
};

struct SymbolTable {
  struct Symbol symbols[MAX_SYMBOLS];
  int count;
};

struct ASTNode * createNumberNode(int value);
struct ASTNode * createIdentifierNode(char *name);
struct ASTNode * createBinaryNode(enum TokenType type, struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * parse_expression(struct Parser *p);
struct ASTNode * parse_primary(struct Parser *p);
struct ASTNode * parse_term(struct Parser *p);
int add_statenent_to_program(struct ASTNode * program, struct ASTNode * statement);

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
    case AST_UNARY:
      return "UNARY";
    case AST_PROGRAM:
      return "PROGRAM";
  }

  return "AST_UNKNOWN";
}

const char *ident_type_name(enum TypeKind type) {
  switch (type) {
    case TYPE_INT:
      return "int";
    default:
      return "unknown";
  }
}

const char *token_type_name(enum TokenType type) {
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

void print_symbol_table(struct SymbolTable *table) {
  struct SymbolTable * t = table;
  if (t == NULL) {
    return;
  }

  printf("Symbol Table\n");
  for (int s = 0; s < t->count; s++) {
    printf("[%d] name=%s, type=%s, value=%d\n",
      s, t->symbols[s].name, ident_type_name(t->symbols[s].type), t->symbols[s].value);
  }
}

void print_ast_tree(struct ASTNode *node, int depth) {
  struct ASTNode * t = node;

  if (t == NULL) {
    return;
  }

  if (t->kind == AST_PROGRAM) {
    printf("%s\n", node_type_name(t->kind));
    for (int c=0; c < t->statement_count; c++) {
      print_ast_tree(t->statements[c], depth+1);
    }
    return;
  } else if (t->kind == AST_ASSIGNMENT) {
    printf("%*s %s\n", SPACES*depth, " ", node_type_name(t->kind));
  } else if (t->kind == AST_NUMBER) {
    printf("%*s %s %d\n", SPACES*depth, " ", node_type_name(t->kind), t->number_value);
  } else if (t->kind == AST_IDENTIFIER) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), t->identifier_name);
  } else if (t->kind == AST_BINARY) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), token_type_name(t->operator_kind));
  } else if (t->kind == AST_UNARY) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), token_type_name(t->operator_kind));
  } else {
    printf("%*s UNKNOWN AST NODE\n", SPACES*depth, " ");
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

struct ASTNode * createUnaryNode(enum TokenType type) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for unary ASTNode\n");
  } else {
    node->kind = AST_UNARY;
    node->number_value = -9999;
    node->left = NULL;
    node->right = NULL;
    node->identifier_name[0] = '\0';
    node->operator_kind = type;
  }
  return node;
}

struct ASTNode * createNumberNode(int value) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for number ASTNode\n");
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
    printf("Couldn't allocate memory for identifier ASTNode\n");
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
    printf("Couldn't allocate memory for binary ASTNode\n");
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
    printf("Couldn't allocate memory for assignment ASTNode\n");
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

// Program node which holds a list of statements and count
struct ASTNode * createProgramNode() {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for program ASTNode\n");
  } else {
    node->kind = AST_PROGRAM;
    node->number_value = -9999;
    node->left = NULL;
    node->right = NULL;
    node->identifier_name[0] = '\0';
    node->operator_kind = TOKEN_EOF;
    node->statements[0] = NULL;
    memset(node->statements, 0, sizeof(node->statements));
    node->statement_count = 0;
  }

  return node;
}

// helper: add new statement to the program node.
void add_statement_to_program(struct ASTNode * program, struct ASTNode * statement) {
  struct ASTNode * pNode = program;

  // we can add few checks
  // program != NULL
  // program->kind == AST_PROGRAM
  // statement != NULL
  if ( (pNode) && (pNode->kind == AST_PROGRAM) && (statement != NULL) ) {
    int count = pNode->statement_count;

    if (count < 256) {
     pNode->statements[count] = statement;
     pNode->statement_count = count + 1;
    }
  }
  return;
}

// Symbol table
void initialize_symbol_table(struct SymbolTable *table) {
  table->count = 0;
}

struct Symbol * lookup_symbol(struct SymbolTable *table, char *name) {
  if (table == NULL) {
    printf("Symbol table pointer in null/invalid\n");
    return NULL;
  }

  for (int i = 0; i < table->count; i++) {
    struct Symbol *symbol = &table->symbols[i];
    if (strcmp(symbol->name, name) == 0) {
      return symbol;
    }
  }

  return NULL;
}

// add symbol if symbol doesn't exists in the table
// update value if symbol exists in the table
void store_symbol(struct SymbolTable *table, char *name, int value) {
  if (table == NULL) {
    printf("Symbol table pointer in null/invalid\n");
    return;
  }

  struct Symbol *symbol = NULL;
  for (int i = 0; i < table->count; i++) {
    symbol = &table->symbols[i];
    if (strcmp(symbol->name, name) == 0) {
      symbol->value = value;
      return;
    }
  }

  if (table->count >= MAX_SYMBOLS) {
    printf("symbol table is full, no space left.\n");
    return;
  }

  size_t symbol_len = strlen(name);
  char *new_symbol = (char *)malloc(symbol_len+1);
  if (new_symbol == NULL) {
    printf("Couldn't allocate memory for new symbol\n");
    return;
  }

  memset(new_symbol, '\0', symbol_len+1);
  strncpy(new_symbol, name, symbol_len);

  table->symbols[table->count].type = TYPE_INT;
  table->symbols[table->count].name = new_symbol;
  table->symbols[table->count].value = value;
  table->count++;

  return;
}

// Evaluator
int eval_expression(struct ASTNode * root, struct SymbolTable *table, int *out_value) {
  struct ASTNode * node = root;

  if (node == NULL) {
    return 0;
  }

  if (node->kind == AST_NUMBER) {
    *out_value = node->number_value;
    return 1;
  } else if (node->kind == AST_UNARY) {
    int result;
    int ret = eval_expression(node->left, table, &result);
    if (ret == 0) {
      return 0; // Failed case
    }
    if (node->operator_kind == TOKEN_MINUS) {
      *out_value = -(result);
    } else if (node->operator_kind == TOKEN_PLUS) {
      *out_value = result;
    }
    return 1;
  } else if (node->kind == AST_BINARY) {
    int leftNum;
    int ret = eval_expression(node->left, table, &leftNum);
    if (ret == 0) {
      return 0; // Failed case
    }
    int rightNum;
    ret = eval_expression(node->right, table, &rightNum);
    if (ret == 0) {
      return 0; // Failed case
    }

    if (node->operator_kind == TOKEN_PLUS) {
      *out_value = leftNum + rightNum;
    } else if (node->operator_kind == TOKEN_MINUS) {
      *out_value = leftNum - rightNum;
    } else if (node->operator_kind == TOKEN_STAR) {
      *out_value = leftNum * rightNum;
    } else if (node->operator_kind == TOKEN_SLASH) {
      *out_value = leftNum / rightNum;
    }
    return 1;
  } else if (node->kind == AST_IDENTIFIER) {
    struct Symbol *symbol = lookup_symbol(table, node->identifier_name);
    if (symbol) {
      *out_value = symbol->value;    
      return 1;
    } else {
      printf("Identifier (%s) has not assigned any value\n", node->identifier_name);
    }
  } else {
    printf("Invalid AST_NODE\n");
  }

  return 0;
}

int eval_statement(struct ASTNode * root, struct SymbolTable *table) {
  struct ASTNode * node = root;
  if (node == NULL) {
    return 0;
  }

  if (node->kind == AST_ASSIGNMENT) {
    char *identifier = node->left->identifier_name;
    int out_value;
    int ret = eval_expression(node->right, table, &out_value);
    if (ret == 0) {
      return 0; // Failed expression evaluation
    }
    printf("%s = %d\n", identifier, out_value);
    // call store_symbol
    store_symbol(table, identifier, out_value);
    return 1;
  }

  return 0;
}

int eval_program(struct ASTNode * root, struct SymbolTable *table) {
  struct ASTNode * node = root;

  if (node == NULL) {
    return 0;
  }

  if (node->kind == AST_PROGRAM) {
    for (int c=0; c < node->statement_count; c++) {
      int result = eval_statement(node->statements[c], table);
      if (result == 0) {
        return 0;
      }
    }
    return 1;
  }

  return 0;
}


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

// unary -> ('-' | '+') unary | primary
struct ASTNode * parse_unary(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  if ( is_current_token(p, TOKEN_MINUS) || is_current_token(p, TOKEN_PLUS) ) {
    struct Token token = current_token(p);
    advance_token(p);
    struct ASTNode * child = parse_unary(p);
    if ( child == NULL ) {
      p->has_error = 1;
      return NULL;
    }

    struct ASTNode * node = createUnaryNode(token.type);
    if (node == NULL) {
      p->has_error = 1;
      return NULL;
    }

    node->left = child;
    return node;
  } else {
    struct ASTNode * node = parse_primary(p);
    return node;
  }

  return NULL;
}

// term -> unary ((STAR | SLASH) unary)*
struct ASTNode * parse_term(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;

  left = parse_unary(p);
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

    struct ASTNode * right = parse_unary(p);
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

// statement -> assignment
struct ASTNode * parse_statement(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }
  struct ASTNode * node = parse_assignment(p);
  return node;
}

// program -> statements*
struct ASTNode * parse_program(struct Parser *p) {

  struct ASTNode * program = createProgramNode();
  if (program == NULL) {
    p->has_error = 1;
    return NULL;
  }

  while ((p->has_error == 0) && (!is_current_token(p, TOKEN_EOF))) {
    struct ASTNode * statement = parse_statement(p);
    if (statement == NULL) {
      p->has_error = 1;
      break;
    }

    // add ASTNode representing a statement to the list (program node)
    add_statement_to_program(program, statement);
  }

  if (p->has_error) {
    return NULL;
  }

  return program;
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
  code="x=(1+2)*3/(4*1);x=1;";
  code="x=-5;";
  code="x=--5;";
  code="x=3*-5;";
  code="x=-(1+2);";
  code="x=-;";
  code="x=-(2;";
  code="x=+5-2;";
  code="x=+(5-2);";
  code="x=-(1+2);";
  code="x=(1+2)*3/(4*1);x=1;";
  code="x=1;";
  code="y=x+2;";
  code="y=6;a=1;b=2;c=a+b;d=a*b+y;";

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

  struct ASTNode * program = parse_program(&p);
  if (p.has_error) {
    printf("Parsing failed\n");
  } else {
    printf("Parsing successful\n");
  }
  printf("Stopped at: ");
  print_token(current_token(&p));
  printf("Print AST Tree:\n ");
  print_ast_tree(program, 0);

  struct SymbolTable table;
  initialize_symbol_table(&table);
  int result = eval_program(program, &table);
  print_symbol_table(&table);
  if (result) {
    printf("SUCCESS: For input program, parse + evaluate / semantic check is okay\n");
  } else {
    printf("ERROR: For input program, parse + evaluate / semantic check failed\n");
  }

  return 0;
}
