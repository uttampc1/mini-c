/*
miniC currently supports only declaration and assignment statements.
miniC currently supports only the int type.
Variables must be declared before they are used.
A variable cannot be redeclared.
The left-hand side of an assignment must be an identifier.
The identifier being assigned to must be already declared.
A declaration may optionally include an initializer expression.
Any identifier used in an expression must be already declared. That covers
- declaration initializer expressions
- assignment expressions
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SUCCESS 1
#define FAIL    0

#define BUFFER_SIZE 256
#define MAX_SYMBOLS 100
#define SPACES 2

enum TokenKind {
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
  TOKEN_INT,
  TOKEN_PRINT,
  TOKEN_UNKNOWN,
  TOKEN_EOF
};

enum ASTNodeKind {
  AST_NUMBER,
  AST_IDENTIFIER,
  AST_BINARY,
  AST_UNARY,
  AST_DECLARATION,
  AST_ASSIGNMENT,
  AST_PRINT,
  AST_PROGRAM
};

enum TypeKind {
  TYPE_INT,
  TYPE_ERROR
};

struct Token {
  enum TokenKind type;
  char *start;
  int  length;
};

struct Parser {
  struct Token *tokens;
  int pos;
  int has_error;
};

struct ASTNode {
  enum ASTNodeKind kind;
  int  number_value;
  char identifier_name[BUFFER_SIZE];
  enum TokenKind operator_kind;
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
struct ASTNode * createBinaryNode(enum TokenKind type, struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createDeclarationNode(enum TokenKind type, struct ASTNode *ident, struct ASTNode *valueNode);
struct ASTNode * parse_expression(struct Parser *p);
struct ASTNode * parse_primary(struct Parser *p);
struct ASTNode * parse_term(struct Parser *p);
int add_statenent_to_program(struct ASTNode * program, struct ASTNode * statement);

enum TypeKind token_to_typekind(enum TokenKind type) {
  if (type == TOKEN_INT) {
    return TYPE_INT;
  }
  return TYPE_ERROR;
}

char *node_type_name(enum ASTNodeKind kind) {
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
    case AST_PRINT:
      return "PRINT";
    case AST_DECLARATION:
      return "DECL";
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

const char *token_type_name(enum TokenKind type) {
  switch (type) {
    case TOKEN_NUMBER:
      return "NUMBER";
    case TOKEN_IDENTIFIER:
      return "IDENTIFIER";
    case TOKEN_INT:
      return "INT";
    case TOKEN_PRINT:
      return "PRINT";
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
void add_token(struct Token tokens[], int *token_count, enum TokenKind type, char *start, int length) {
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
  } else if (t->kind == AST_DECLARATION) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), token_type_name(t->operator_kind));
  } else if (t->kind == AST_PRINT) {
    printf("%*s %s\n", SPACES*depth, " ", node_type_name(t->kind));
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
      if (strncmp(code+start, "int", idx-start) == 0) {
        add_token(tokens, token_count, TOKEN_INT, code + start, idx - start);
      } else if (strncmp(code+start, "print", idx-start) == 0) {
        add_token(tokens, token_count, TOKEN_PRINT, code + start, idx - start);
      } else {
        add_token(tokens, token_count, TOKEN_IDENTIFIER, code + start, idx - start);
      }
    } else {
      add_token(tokens, token_count, TOKEN_UNKNOWN, code + idx, 1);
      idx++;
    }
  }
  add_token(tokens, token_count, TOKEN_EOF, code + idx, 0);
}

// Parser
int is_current_token(struct Parser *p, enum TokenKind type) {
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

void expect_token(struct Parser *p, enum TokenKind type) {
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

struct ASTNode * createUnaryNode(enum TokenKind type) {
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

struct ASTNode * createDeclarationNode(enum TokenKind type, struct ASTNode *ident, struct ASTNode * valueNode) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for identifier ASTNode\n");
    return NULL;
  }

  node->kind = AST_DECLARATION;
  node->number_value = -99999;
  node->left = ident;
  node->right = valueNode;
  node->identifier_name[0] = '\0';
  node->operator_kind = type;
  return node;
}

struct ASTNode * createBinaryNode(enum TokenKind type, struct ASTNode * leftNode, struct ASTNode * rightNode) {
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

// Print node which holds a expression subtree
struct ASTNode * createPrintNode(struct ASTNode * expr) {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    printf("Couldn't allocate memory for print ASTNode\n");
  } else {
    node->kind = AST_PRINT;
    node->number_value = -9999;
    node->left = expr;
    node->right = NULL;
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
    return FAIL;
  }

  if (node->kind == AST_NUMBER) {
    *out_value = node->number_value;
    return SUCCESS;
  } else if (node->kind == AST_UNARY) {
    int result;
    int ret = eval_expression(node->left, table, &result);
    if (ret == FAIL) {
      return FAIL; // Failed case
    }
    if (node->operator_kind == TOKEN_MINUS) {
      *out_value = -(result);
    } else if (node->operator_kind == TOKEN_PLUS) {
      *out_value = result;
    }
    return SUCCESS;
  } else if (node->kind == AST_BINARY) {
    int leftNum;
    int ret = eval_expression(node->left, table, &leftNum);
    if (ret == FAIL) {
      return FAIL; // Failed case
    }
    int rightNum;
    ret = eval_expression(node->right, table, &rightNum);
    if (ret == FAIL) {
      return FAIL; // Failed case
    }

    if (node->operator_kind == TOKEN_PLUS) {
      *out_value = leftNum + rightNum;
    } else if (node->operator_kind == TOKEN_MINUS) {
      *out_value = leftNum - rightNum;
    } else if (node->operator_kind == TOKEN_STAR) {
      *out_value = leftNum * rightNum;
    } else if (node->operator_kind == TOKEN_SLASH) {
      if (rightNum == 0) {
        printf("error: division by zero.\n");
        return FAIL;
      }
      *out_value = leftNum / rightNum;
    }
    return SUCCESS;
  } else if (node->kind == AST_IDENTIFIER) {
    struct Symbol *symbol = lookup_symbol(table, node->identifier_name);
    if (symbol) {
      //printf("Symbol(%s) found with value(%d)\n", symbol->name, symbol->value);
      *out_value = symbol->value;    
      return SUCCESS;
    } else {
      printf("error: undefined identifier (%s)\n", node->identifier_name);
    }
  } else {
    printf("Invalid AST_NODE\n");
  }

  return FAIL;
}

int eval_statement(struct ASTNode * root, struct SymbolTable *table) {
  struct ASTNode * node = root;
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_DECLARATION) {
    char *identifier = node->left->identifier_name;
    int out_value = 0;
    struct Symbol *symbol = lookup_symbol(table, identifier);
    if (symbol) {
      printf("error: redeclaration of identifier %s\n", identifier);
      return FAIL;
    }
    if (node->right) {
      int ret = eval_expression(node->right, table, &out_value);
      if (ret == FAIL) {
       return FAIL; // Failed expression evaluation
      }
    }
    printf("declare %s with initial value %d\n", identifier, out_value);
    store_symbol(table, identifier, out_value);
    return SUCCESS;
  } else if (node->kind == AST_ASSIGNMENT) {
    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol(table, identifier);
    if (symbol == NULL) {
      printf("error: assignment to undeclared identifier %s\n", identifier);
      return FAIL;
    } else {
      int out_value;
      int ret = eval_expression(node->right, table, &out_value);
      if (ret == FAIL) {
       return FAIL; // Failed expression evaluation
      }
      printf("%s = %d\n", identifier, out_value);
      // call store_symbol
      store_symbol(table, identifier, out_value);
    }
    return SUCCESS;
  } else if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      printf("error: print statement missing expression\n");
      return FAIL;
    }

    int out_value;
    int ret = eval_expression(node->left, table, &out_value);
    if (ret == FAIL) {
      return FAIL;
    }

    printf("%d\n", out_value);
    return SUCCESS; // print statment executed successfully.
  }

  return FAIL;
}

int eval_program(struct ASTNode * root, struct SymbolTable *table) {
  struct ASTNode * node = root;

  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_PROGRAM) {
    for (int c=0; c < node->statement_count; c++) {
      if(eval_statement(node->statements[c], table) == FAIL) {
        return FAIL;
      }
    }
    return SUCCESS;
  }

  return FAIL;
}

// type check over AST - NO COMPUTE
enum TypeKind analyze_expression_type(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    return TYPE_ERROR;
  }

  if (node->kind == AST_NUMBER) {
    return TYPE_INT;
  } else if (node->kind == AST_IDENTIFIER) {
    char *identifier = node->identifier_name;
    struct Symbol *symbol = lookup_symbol(table, identifier);
    if (symbol == NULL) {
      return TYPE_ERROR;
    }
    return symbol->type;
  } else if (node->kind == AST_BINARY) {
    enum TypeKind leftType = analyze_expression_type(node->left, table);
    if (leftType == TYPE_ERROR) {
      printf("error: left expression has unsupported type\n");
      return TYPE_ERROR;
    }
    enum TypeKind rightType = analyze_expression_type(node->right, table);
    if (rightType == TYPE_ERROR) {
      printf("error: right expression has unsupported type\n");
      return TYPE_ERROR;
    }
    if (leftType != rightType) {
      printf("error: binary expression operands have different Types\n");
      return TYPE_ERROR;
    }

    return leftType;
  } else if (node->kind == AST_UNARY) {
    if (node->left == NULL) {
      printf("error: unary expression missing operand\n");
      return TYPE_ERROR;
    }

    enum TypeKind operand_type = analyze_expression_type(node->left, table);
    if (operand_type == TYPE_ERROR || operand_type != TYPE_INT) {
      printf("error: expression has unsupported type\n");
      return TYPE_ERROR;
    }

    return operand_type;
  } 

  printf("error: unsupported expression for type check analysis: %s\n", node_type_name(node->kind));
  return TYPE_ERROR;
}

// semantic analysis pass over AST - NO COMPUTE
// - redeclaration is not allowed
// - assignment target must already be declared
// - identifiers used in expression must already be declared
int analyze_expression(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_NUMBER) {
    return SUCCESS; // if number okay
  } else if (node->kind == AST_UNARY) {
    if (node->left == NULL) {
      printf("error: unary expression missing operand\n");
      return FAIL;
    };

    if (node->operator_kind != TOKEN_MINUS && node->operator_kind != TOKEN_PLUS) {
      printf("error: unsupported unary operator %s\n", token_type_name(node->operator_kind));
      return FAIL;
    }
    int ret = analyze_expression(node->left, table);
    if (ret == FAIL) {
      return FAIL;
    }
    return SUCCESS;
  } else if (node->kind == AST_IDENTIFIER) {
    char *identifier = node->identifier_name;
    struct Symbol *symbol = lookup_symbol(table, identifier);
    if (symbol == NULL) {
      printf("error: use of undeclared identifier %s\n", identifier);
      return FAIL;
    }
    return SUCCESS; // if identifier exists in the symbol table. okay
  } else if (node->kind == AST_BINARY) {
    int ret = analyze_expression(node->left, table);
    if (ret == FAIL) {
      return FAIL; // Failed case
    }
    ret = analyze_expression(node->right, table);
    if (ret == FAIL) {
      return FAIL; // Failed case
    }
    return SUCCESS; // if left and right child semantically okay
  }

  printf("error: unsupported expression ASTNode: %s\n", node_type_name(node->kind));
  return FAIL;
}

int analyze_statement(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_DECLARATION) {
    enum TypeKind declared_type = token_to_typekind(node->operator_kind);

    if (declared_type == TYPE_ERROR) {
      printf("error: unsupported declaration type %s\n",
        token_type_name(node->operator_kind));
      return FAIL;
    }

    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("error: %s declaration must have an identifier\n",
        token_type_name(node->operator_kind));
      return FAIL;
    }

    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol(table, identifier);
    if (symbol) {
      printf("error: redeclaration of identifier %s\n", identifier);
      return FAIL;
    }
    store_symbol(table, identifier, 0);

    if (node->right) {
      enum TypeKind expression_type = analyze_expression_type(node->right, table);
      if (expression_type == TYPE_ERROR) {
        printf("error: RHS expression has unsupported type.\n");
        return FAIL;
      }

      if (declared_type != expression_type) {
        printf("error: intializer assignment type mismatch for declared identifier %s\n", identifier);
        return FAIL;
      }

      int ret = analyze_expression(node->right, table);
      if (ret == FAIL) {
        return FAIL;
      }
    }
    return SUCCESS;
  } else  if (node->kind == AST_ASSIGNMENT) {
    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("error: assignment left side must be an identifier\n");
      return FAIL;
    }

    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol(table, identifier);
    if (symbol == NULL) {
      printf("error: assignment to undeclared identifier %s\n", identifier);
      return FAIL;
    }

    if (node->right == NULL) {
      printf("error: missing assignment value for identifier %s\n", identifier);
      return FAIL;
    }

    enum TypeKind expr_type = analyze_expression_type(node->right, table);
    if (expr_type == TYPE_ERROR) {
      printf("error: RHS expression has unsupported type.\n");
      return FAIL;
    }

    if (symbol->type != expr_type) {
      printf("error: assignment type mismatch for identifier %s\n", identifier);
      return FAIL;
    }

    int ret = analyze_expression(node->right, table);
    if (ret == FAIL) {
      return FAIL;
    }

    return SUCCESS;
  } else  if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      return SUCCESS; // There is nothing to print and that's okay.
    }

    int ret = analyze_expression(node->left, table);
    if (ret == FAIL) {
      return FAIL;
    }

    enum TypeKind expr_type = analyze_expression_type(node->left, table);
    if (expr_type == TYPE_ERROR) {
      printf("error: print expression has unsupported type\n");
      return FAIL;
    }

    return SUCCESS;
  }

  printf("error: unsupported statement ASTNode: %s\n", node_type_name(node->kind));
  return FAIL;
}

int analyze_program(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_PROGRAM) {
    for (int c=0; c < node->statement_count; c++) {
      if (analyze_statement(node->statements[c], table) == FAIL) { 
        return FAIL;
      }
    }
    return SUCCESS;
  }

  return FAIL;
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

// declaration -> INT IDENTIFIER SEMICOLON
//              | INT IDENTIFIER EQUAL expression SEMICOLON
// OR
// declaration -> INT IDENTIFIER ("=" expression)? SEMICOLON -- compact grammer version
struct ASTNode * parse_declaration(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  expect_token(p, TOKEN_INT);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * ident = parse_identifier(p);
  if ( ident == NULL ) {
    p->has_error = 1;
    return NULL;
  }

  struct ASTNode * rightValue = NULL;
  if (is_current_token(p, TOKEN_EQUAL)) {
    expect_token(p, TOKEN_EQUAL);
    rightValue = parse_expression(p);
  }

  expect_token(p, TOKEN_SEMICOLON);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * node = createDeclarationNode(TOKEN_INT, ident, rightValue);
  if (node == NULL) {
    p->has_error = 1;
  }

  return node;
}

// print_statement -> PRINT LPAREN expression RPAREN SEMICON
struct ASTNode * parse_print_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  expect_token(p, TOKEN_PRINT);
  if ( p->has_error ) {
    return NULL;
  }

  if (is_current_token(p, TOKEN_LPAREN)) {
    parse_lparen(p);
    struct ASTNode * expr = parse_expression(p);
    if (expr == NULL) {
      p->has_error = 1;
    } else {
      parse_rparen(p);
    }

    expect_token(p, TOKEN_SEMICOLON);
    if ( p->has_error ) {
     return NULL;
    }

    struct ASTNode * node = createPrintNode(expr);
    if (node == NULL) {
      p->has_error = 1;
    }
    return node;
  }

  return NULL;
}

// statement -> declaration | assignment | print_statement
struct ASTNode * parse_statement(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_INT)) {
    node = parse_declaration(p);
  } else if (is_current_token(p, TOKEN_PRINT)) {
    node = parse_print_stmt(p);
  } else {
    node = parse_assignment(p);
  }
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
  code="y=6;a=1;b=2;c=a+b;d=a*b+z;";
  code="x=0;y=2/x;";
  code="int x;";
  code="int x=5;";
  code="int x; x=5+4; int y=2; int z=x+y;";
  code="int x; int x;";
  code="int z = x + y; int x = 1; int y = 1;";
  code="int x = 5; int y = x + 2;";
  code="int x=1; int y=x;";
  code="print(x+y);";
  code="int x=10; x=3; int y=5;print(x+y);print(6+9);";
  code="int x; int x=5;";
  code="int x=40; int y=2; print(x+x/y); print(x+2*y); print(x);";
  code="print(20-5-3);";
  code="print(20/5/2);";
  code="int x=4; int y=2; print((x/x)/y);";
  //code="int x=4; int y=2; print((x+2)*y);";
  code="print(10/0);";
  code="int x=-5;print(x);";
  code="int x=1; print(-5); print(-(4+3));print(-x);";

  printf("Input: %s\n", code);
  // Lexer
  printf("\n\n--> Tokenizer PHASE: Start\n");
  tokenize(code, tokens, &token_count);
  printf("--> Print tokens--\n");
  for (int i = 0; i < token_count; i++) {
    print_token(tokens[i]);
  }

  // Parser
  struct Parser p;
  p.tokens = tokens;
  p.pos = 0;
  p.has_error = 0;

  printf("\n\n--> PARSING PHASE: Start\n");
  struct ASTNode * program = parse_program(&p);
  if (p.has_error) {
    printf("--> PARSING PHASE: FAILED\n");
    exit(-1);
  }
  printf("--> PARSING PHASE: PASS\n");

  printf("Stopped at: ");
  print_token(current_token(&p));
  printf("Print AST Tree:\n ");
  print_ast_tree(program, 0);

  // Check sematics of the program before we evaluate it
  printf("\n\n--> SEMANTIC CHECK PHASE: Start\n");
  printf("Initialize symbol table for Semantic check\n");
  struct SymbolTable semantic_table;
  initialize_symbol_table(&semantic_table);

  int result = analyze_program(program, &semantic_table);
  print_symbol_table(&semantic_table);
  if (result == 0) {
    printf("--> SEMANTIC CHECK PHASE: FAILED\n");
    exit(-1);
  }
  printf("--> SEMANTIC CHECK PHASE: PASS\n");

  // Program is semantically okay so go ahead and evaluate it
  printf("\n\n--> EVALUATION CHECK PHASE: Start\n");
  printf("Initialize symbol table for Evaluation check\n");
  struct SymbolTable runtime_table;
  initialize_symbol_table(&runtime_table);
  result = eval_program(program, &runtime_table);
  print_symbol_table(&runtime_table);
  if (result == 0) {
    printf("--> EVALUATION CHECK PHASE: FAILED\n");
    exit(-1);
  }
  printf("--> EVALUATION CHECK PHASE: PASS\n");

  return 0;
}
