/******************************************************************************
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
- print_statment (this is explicit AST Node to print result

=============
GRAMMER rules
=============
program -> statements*
statement -> declaration | assignment | print_statement
declaration -> INT IDENTIFIER ("=" expression)? SEMICOLON
assignment -> IDENTIFIER EQUAL expression SEMICOLON
print_statement -> PRINT LPAREN expression RPAREN SEMICOLON

expression -> equality
equality -> comparison ((EQUAL_EQUAL | BANG_EQUAL) comparison)*
comparison -> term ((LESS | GREATER) term)*
term -> factor ((PLUS | MINUS) factor)*
factor -> unary ((STAR | SLASH) unary)*
unary -> ('-' | '+') unary | primary
primary -> NUMBER | IDENTIFIER | LPAREN expression RPAREN


******************************************************************************/


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
  TOKEN_LESS,
  TOKEN_GREATER,
  TOKEN_EQUAL_EQUAL,
  TOKEN_BANG_EQUAL,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_SEMICOLON,
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
  AST_IF,
  AST_BLOCK,
  AST_PROGRAM,
  AST_UNKNOWN
};

enum TypeKind {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_UNKNOWN,
  TYPE_UNUSED,
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
  enum   ASTNodeKind kind;
  int    number_value;
  double float_number_value;
  char   identifier_name[BUFFER_SIZE];
  enum   TokenKind operator_kind;
  enum   TypeKind  type_kind;
  enum   TypeKind  declared_variable_type;
  struct ASTNode * left;
  struct ASTNode * right;
  struct ASTNode * then_branch;
  struct ASTNode * else_branch;
  struct ASTNode * statements[BUFFER_SIZE];
  int    statement_count;
};

struct Value {
  enum   TypeKind type;
  int    int_value;
  double float_value;
};

struct Symbol {
  enum TypeKind type;
  char *name;
  struct Value value;
};

struct SymbolTable {
  struct SymbolTable *parent;
  struct Symbol symbols[MAX_SYMBOLS];
  int count;
};

struct ASTNode * createASTNode();
struct ASTNode * createUnaryNode(enum TokenKind type);
struct ASTNode * createNumberNode(enum TypeKind type, char * value);
struct ASTNode * createIdentifierNode(char *name);
struct ASTNode * createBinaryNode(enum TokenKind type, struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createDeclarationNode(enum TypeKind declaredType, struct ASTNode *ident, struct ASTNode *valueNode);
struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createPrintNode(struct ASTNode * expr);
struct ASTNode * createIfNode(struct ASTNode * expr, struct ASTNode * then_stmt, struct ASTNode * else_stmt);
struct ASTNode * createBlockNode();
struct ASTNode * createProgramNode();

struct Symbol  * lookup_symbol_current(struct SymbolTable *table, char *name); // current scope search
struct Symbol  * lookup_symbol_visible(struct SymbolTable *table, char *name); // globalc scope search
struct ASTNode * parse_program(struct Parser *p);
struct ASTNode * parse_block(struct Parser *p);
struct ASTNode * parse_statement(struct Parser *p);
struct ASTNode * parse_expression(struct Parser *p);
struct ASTNode * parse_primary(struct Parser *p);
struct ASTNode * parse_term(struct Parser *p);

void add_statement_to_program(struct ASTNode * program, struct ASTNode * statement);
void add_statement_to_block(struct ASTNode * node, struct ASTNode * statement);
int is_assignment_compatible(enum TypeKind target_type, enum TypeKind source_type);

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
    case AST_BLOCK:
      return "BLOCK";
    case AST_IF:
      return "IF";
    case AST_PROGRAM:
      return "PROGRAM";
  }

  return "AST_UNKNOWN";
}

const char *typekind_type_name(enum TypeKind type) {
  switch (type) {
    case TYPE_INT:
      return "INT";
    case TYPE_FLOAT:
      return "FLOAT";
    case TYPE_UNKNOWN:
      return "UNKNOWN";
    case TYPE_UNUSED:
      return "UNUSED";
    default:
      return "TYPE_ERROR";
  }
}

const char *token_type_name(enum TokenKind type) {
  switch (type) {
    case TOKEN_NUMBER:
      return "NUMBER";
    case TOKEN_IDENTIFIER:
      return "IDENTIFIER";
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
    case TOKEN_LESS:
      return "<";
    case TOKEN_GREATER:
      return ">";
    case TOKEN_EQUAL_EQUAL:
      return "==";
    case TOKEN_BANG_EQUAL:
      return "!=";
    case TOKEN_LPAREN:
      return "(";
    case TOKEN_RPAREN:
      return ")";
    case TOKEN_LBRACE:
      return "LBRACE";
    case TOKEN_RBRACE:
      return "RBRACE";
    case TOKEN_INT:
      return "INT";
    case TOKEN_FLOAT:
      return "FLOAT";
    case TOKEN_IF:
      return "IF";
    case TOKEN_ELSE:
      return "ELSE";
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

  printf("--- print Symbol Table ---\n");
  if (t->count <= 0) {
    //printf("warning: no symbols found\n");
    return;
  }

  for (int s = 0; s < t->count; s++) {
    printf("[%d] name=%s, type=%s ", s, t->symbols[s].name, typekind_type_name(t->symbols[s].type));
    if (t->symbols[s].value.type == TYPE_INT) {
      printf("value=%d\n", t->symbols[s].value.int_value);
    } else if (t->symbols[s].value.type == TYPE_FLOAT) {
      printf("value=%f\n", t->symbols[s].value.float_value);
    } else {
      printf("\n");
    }
  }

  return;
}

void print_ast_tree(struct ASTNode *node, int depth) {
  struct ASTNode * t = node;

  if (t == NULL) {
    return;
  }

  if (t->kind == AST_PROGRAM) {
    printf("%s %s\n", node_type_name(t->kind), typekind_type_name(t->type_kind));
    for (int c=0; c < t->statement_count; c++) {
      print_ast_tree(t->statements[c], depth+1);
    }
    return;
  } else if (t->kind == AST_DECLARATION) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_BLOCK) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), typekind_type_name(t->type_kind));
    for (int c=0; c < t->statement_count; c++) {
      print_ast_tree(t->statements[c], depth+1);
    }
  } else if (t->kind == AST_IF) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->left != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "CONDITION");
      print_ast_tree(t->left, depth+2);
    }

    if (t->then_branch != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "THEN");
      print_ast_tree(t->then_branch, depth+2);
    }

    if (t->else_branch != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "ELSE");
      print_ast_tree(t->else_branch, depth+2);
    }
    return;
  } else if (t->kind == AST_PRINT) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_ASSIGNMENT) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_NUMBER) {
    if (t->type_kind == TYPE_INT) {
      printf("%*s %s %d %s\n", SPACES*depth, " ", node_type_name(t->kind), t->number_value, typekind_type_name(t->type_kind));
    } else  if (t->type_kind == TYPE_FLOAT) {
      printf("%*s %s %f %s\n", SPACES*depth, " ", node_type_name(t->kind), t->float_number_value, typekind_type_name(t->type_kind));
    }
  } else if (t->kind == AST_IDENTIFIER) {
    printf("%*s %s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), t->identifier_name, typekind_type_name(t->type_kind));
  } else if (t->kind == AST_BINARY) {
    printf("%*s %s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), token_type_name(t->operator_kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_UNARY) {
    printf("%*s %s %s\n", SPACES*depth, " ", node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else {
    printf("%*s UNKNOWN AST NODE\n", SPACES*depth, " ");
  }

  print_ast_tree(t->left, depth+1);
  print_ast_tree(t->right, depth+1);
  print_ast_tree(t->else_branch, depth+1);

  return;
}

int is_alpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

int is_equal(char c) {
  return (c == '=');
}

int is_less(char c) {
  return (c == '<');
}

int is_greater(char c) {
  return (c == '>');
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

int is_lbrace(char c) {
  return (c == '{');
}

int is_rbrace(char c) {
  return (c == '}');
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

  if (code[i] == '.') {
    i++;

    while (is_digit(code[i])) {
      i++;
    }
  }

  return i;
}

void skip_spaces(char *code, int *i) {
  while(is_space(code[*i])) {
    *i = *i + 1;
  }
}

int tokenize(char *code, struct Token tokens[], int *token_count) {
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
      if (code[idx+1] == '=') {
        add_token(tokens, token_count, TOKEN_EQUAL_EQUAL, code + idx, 2);
        idx += 2;
      } else {
        add_token(tokens, token_count, TOKEN_EQUAL, code + idx, 1);
        idx++;
      }
    } else if (is_less(code[idx])) {
      add_token(tokens, token_count, TOKEN_LESS, code + idx, 1);
      idx++;
    } else if (is_greater(code[idx])) {
      add_token(tokens, token_count, TOKEN_GREATER, code + idx, 1);
      idx++;
    } else if (code[idx] == '!') {
      if (code[idx + 1] == '=') {
        add_token(tokens, token_count, TOKEN_BANG_EQUAL, code + idx, 2);
        idx += 2;
      } else {
        printf("error: unexpected character '!'\n");
        return FAIL;
      }
    } else if (is_lparen(code[idx])) {
      add_token(tokens, token_count, TOKEN_LPAREN, code + idx, 1);
      idx++;
    } else if (is_rparen(code[idx])) {
      add_token(tokens, token_count, TOKEN_RPAREN, code + idx, 1);
      idx++;
    } else if (is_lbrace(code[idx])) {
      add_token(tokens, token_count, TOKEN_LBRACE, code + idx, 1);
      idx++;
    } else if (is_rbrace(code[idx])) {
      add_token(tokens, token_count, TOKEN_RBRACE, code + idx, 1);
      idx++;
    } else if (is_alpha(code[idx])) {
      int start = idx;
      idx = scan_identifier(code, idx);
      // keywords
      if (strncmp(code+start, "int", idx-start) == 0) {
        add_token(tokens, token_count, TOKEN_INT, code + start, idx - start);
      } else if (strncmp(code+start, "float", idx-start) == 0) {
        add_token(tokens, token_count, TOKEN_FLOAT, code + start, idx - start);
      } else if (strncmp(code+start, "if", idx-start) == 0) {
        add_token(tokens, token_count, TOKEN_IF, code + start, idx - start);
      } else if (strncmp(code+start, "else", idx-start) == 0) {
        add_token(tokens, token_count, TOKEN_ELSE, code + start, idx - start);
      } else if (strncmp(code+start, "print", idx-start) == 0) {
        // internal print() funtion/keyword
        add_token(tokens, token_count, TOKEN_PRINT, code + start, idx - start);
      } else {
        // identifiers
        add_token(tokens, token_count, TOKEN_IDENTIFIER, code + start, idx - start);
      }
    } else {
      add_token(tokens, token_count, TOKEN_UNKNOWN, code + idx, 1);
      idx++;
    }
  }

  add_token(tokens, token_count, TOKEN_EOF, code + idx, 0);
  return SUCCESS;
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
    advance_token(p);
  } else {
    p->has_error = 1;
    printf("error: expected %s token, found %s token instead\n", token_type_name(type), token_type_name(t.type));
  }
  return;
}

// Generic AST Node allocation
struct ASTNode * createASTNode() {
  struct ASTNode * node = NULL;
  node = (struct ASTNode *)malloc(sizeof(struct ASTNode));
  if (node == NULL) {
    return NULL;
  }

  node->kind = AST_UNKNOWN;
  node->number_value = -9999;
  node->float_number_value = -9999.9999;
  node->left = NULL;
  node->right = NULL;
  node->then_branch = NULL;
  node->else_branch = NULL;
  node->identifier_name[0] = '\0';
  node->operator_kind = TOKEN_UNKNOWN;
  node->type_kind = TYPE_UNUSED;
  node->declared_variable_type = TYPE_UNUSED;
  for (int i=0; i < BUFFER_SIZE; i++) {
    node->statements[i] = NULL;
  }
  node->statement_count = 0;
  return node;
}

// AST_UNARY node allocation
struct ASTNode * createUnaryNode(enum TokenKind type) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for unary ASTNode\n");
    return NULL;
  }

  node->kind = AST_UNARY;
  node->operator_kind = type;
  return node;
}

struct ASTNode * createNumberNode(enum TypeKind type, char * value) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for number ASTNode\n");
    return NULL;
  }

  node->kind = AST_NUMBER;
  if (type == TYPE_INT) {
    node->type_kind = TYPE_INT;
    //node->number_value = strtol(value, NULL, 10);
    node->number_value = atoi(value);
  } else  if (type == TYPE_FLOAT) {
    node->type_kind = TYPE_FLOAT;
    //node->float_number_value = strtod(value, NULL);
    node->float_number_value = atof(value);
  }
  return node;
}

struct ASTNode * createIdentifierNode(char *name) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for identifier ASTNode\n");
    return NULL;
  }

  node->kind = AST_IDENTIFIER;
  size_t len = strlen(name);
  int bytesToCopy = (len >= BUFFER_SIZE) ? BUFFER_SIZE-1 : len;
  strncpy(node->identifier_name, name, bytesToCopy);
  node->identifier_name[bytesToCopy] = '\0';
  return node;
}

struct ASTNode * createBinaryNode(enum TokenKind type, struct ASTNode * leftNode, struct ASTNode * rightNode) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for binary ASTNode\n");
    return NULL;
  }

  node->kind = AST_BINARY;
  node->left = leftNode;
  node->right = rightNode;
  node->operator_kind = type;
  return node;
}

struct ASTNode * createDeclarationNode(enum TypeKind declaredType, struct ASTNode *ident, struct ASTNode * valueNode) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for identifier ASTNode\n");
    return NULL;
  }

  node->kind = AST_DECLARATION;
  node->left = ident;
  node->right = valueNode;
  node->declared_variable_type = declaredType;
  return node;
}

struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for assignment ASTNode\n");
    return NULL;
  }

  node->kind = AST_ASSIGNMENT;
  node->left = leftNode;
  node->right = rightNode;
  return node;
}

// Print node which holds a expression subtree
struct ASTNode * createPrintNode(struct ASTNode * expr) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for print ASTNode\n");
    return NULL;
  }

  node->kind = AST_PRINT;
  node->left = expr;
  return node;
}

// BLOCK node which holds a list of statements for a block and count
struct ASTNode * createBlockNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for block ASTNode\n");
    return NULL;
  }

  node->kind = AST_BLOCK;
  return node;
}

struct ASTNode * createIfNode(struct ASTNode * expr, struct ASTNode * then_stmt, struct ASTNode * else_stmt) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for if ASTNode\n");
    return NULL;
  }

  node->kind = AST_IF;
  node->left = expr;   // condition. We will add may be "expr" pointer later on
  node->then_branch = then_stmt;
  node->else_branch = else_stmt;
  return node;
}

// Program node which holds a list of statements and count
struct ASTNode * createProgramNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for program ASTNode\n");
    return NULL;
  }

  node->kind = AST_PROGRAM;
  return node;
}


// helper: add new statement to the program node.
void add_statement_to_program(struct ASTNode * node, struct ASTNode * statement) {
  struct ASTNode * program = node;

  // we can add few checks
  // program != NULL
  // program->kind == AST_PROGRAM
  // statement != NULL
  if ( program && (program->kind == AST_PROGRAM) && (statement != NULL) ) {
    int count = program->statement_count;

    if (count < 256) {
     program->statements[count] = statement;
     program->statement_count = count + 1;
    }
  }
  return;
}

void add_statement_to_block(struct ASTNode * node, struct ASTNode * statement) {
  struct ASTNode * block = node;

  // we can add few checks
  // program != NULL
  // program->kind == AST_PROGRAM
  // statement != NULL
  if ( block && (block->kind == AST_BLOCK) && (statement != NULL) ) {
    int count = block->statement_count;

    if (count < 256) {
     block->statements[count] = statement;
     block->statement_count = count + 1;
    }
  }
  return;
}

// Symbol table
void initialize_symbol_table(struct SymbolTable *table, struct SymbolTable *parent) {
  table->parent = parent;
  table->count = 0;
}

// return symbol if it exists in current scope
struct Symbol * lookup_symbol_current(struct SymbolTable *table, char *name) {
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

// if current scope doesn't have, check its parent, until
// either it is found or parent is NULL.
struct Symbol * lookup_symbol_visible(struct SymbolTable *table, char *name) {
  if (table == NULL) {
    printf("Symbol table pointer in null/invalid\n");
    return NULL;
  }

  // search current scope (table)
  struct Symbol * symbol = lookup_symbol_current(table, name);
  if (symbol) {
    return symbol;
  }

  // not found in current, check it's parent recursively
  if (table->parent) {
    return lookup_symbol_visible(table->parent, name);
  }

  return NULL;
}

// add symbol if symbol doesn't exists in the table
// update value if symbol exists in the table
void store_symbol(struct SymbolTable *table, enum TypeKind type, char *name, struct Value *value) {
  //printf("Add [%s] symbol to the table\n", name);
  if (table == NULL) {
    printf("Symbol table pointer in null/invalid\n");
    return;
  }

  struct Symbol *symbol = NULL;
  for (int i = 0; i < table->count; i++) {
    symbol = &table->symbols[i];
    if (strcmp(symbol->name, name) == 0) {
      symbol->value = *value;
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
    printf("error: couldn't allocate memory for new symbol\n");
    return;
  }

  for (int i=0; i < symbol_len+1; i++) {
    new_symbol[i] = '\0';
  }
  strncpy(new_symbol, name, symbol_len);

  table->symbols[table->count].type = type;
  table->symbols[table->count].name = new_symbol;
  table->symbols[table->count].value = *value;
  table->count++;

  return;
}

// Evaluator
int eval_expression(struct ASTNode * node, struct SymbolTable *table, struct Value *out_value) {

  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_NUMBER) {
    out_value->type = node->type_kind;
    if (node->type_kind == TYPE_INT) {
      out_value->int_value = node->number_value;
    } else if (node->type_kind == TYPE_FLOAT) {
      out_value->float_value = node->float_number_value;
    } else {
      return FAIL;
    }
    return SUCCESS;
  } else if (node->kind == AST_UNARY) {
    struct Value result;
    int ret = eval_expression(node->left, table, &result);
    if (ret == FAIL) {
      return FAIL;
    }

    out_value->type = result.type;

    if (node->operator_kind == TOKEN_MINUS) {
      if (result.type == TYPE_INT) {
        out_value->int_value = -(result.int_value);
      } else if (result.type == TYPE_FLOAT) {
        out_value->float_value = -(result.float_value);
      } else {
        return FAIL;
      }
    } else if (node->operator_kind == TOKEN_PLUS) {
      if (result.type == TYPE_INT) {
        out_value->int_value = result.int_value;
      } else if (result.type == TYPE_FLOAT) {
        out_value->float_value = result.float_value;
      } else {
        return FAIL;
      }
    } else {
      printf("error: unsupported unary operator\n");
      return FAIL;
    }
    return SUCCESS;
  } else if (node->kind == AST_BINARY) {
    struct Value leftNum;
    int ret = eval_expression(node->left, table, &leftNum);
    if (ret == FAIL) {
      return FAIL;
    }
    struct Value rightNum;
    ret = eval_expression(node->right, table, &rightNum);
    if (ret == FAIL) {
      return FAIL;
    }

    // int op int -> int
    // float op float -> float
    // float op int OR int op float -> promote to float
    enum TokenKind operator_kind =  node->operator_kind;

    if (operator_kind == TOKEN_LESS || operator_kind == TOKEN_GREATER
      || operator_kind == TOKEN_EQUAL_EQUAL || operator_kind == TOKEN_BANG_EQUAL) {
      out_value->type = TYPE_INT;
    }

    if (leftNum.type == TYPE_INT && rightNum.type == TYPE_INT) {
      out_value->type = TYPE_INT;
      out_value->float_value = 0.0;
      if (operator_kind == TOKEN_PLUS) {
        out_value->int_value = leftNum.int_value + rightNum.int_value;
      } else if (operator_kind == TOKEN_MINUS) {
        out_value->int_value = leftNum.int_value - rightNum.int_value;
      } else if (operator_kind == TOKEN_STAR) {
        out_value->int_value = leftNum.int_value * rightNum.int_value;
      } else if (operator_kind == TOKEN_SLASH) {
        if (rightNum.int_value == 0) {
          printf("error: division by zero.\n");
          return FAIL;
        }
        out_value->int_value = leftNum.int_value / rightNum.int_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->int_value = (leftNum.int_value < rightNum.int_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->int_value = (leftNum.int_value > rightNum.int_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->int_value = (leftNum.int_value == rightNum.int_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->int_value = (leftNum.int_value != rightNum.int_value);
      } else {
        printf("error: unsupported integer binary operator.\n");
        return FAIL;
      }
    } else if (leftNum.type == TYPE_FLOAT && rightNum.type == TYPE_FLOAT) {
      out_value->type = TYPE_FLOAT;
      out_value->int_value = 0;
      out_value->float_value = 0.0;
      if (operator_kind == TOKEN_PLUS) {
        out_value->float_value = leftNum.float_value + rightNum.float_value;
      } else if (operator_kind == TOKEN_MINUS) {
        out_value->float_value = leftNum.float_value - rightNum.float_value;
      } else if (operator_kind == TOKEN_STAR) {
        out_value->float_value = leftNum.float_value * rightNum.float_value;
      } else if (operator_kind == TOKEN_SLASH) {
        if (rightNum.float_value == 0) {
          printf("error: division by zero.\n");
          return FAIL;
        }
        out_value->float_value = leftNum.float_value / rightNum.float_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value < rightNum.float_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value > rightNum.float_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value == rightNum.float_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value != rightNum.float_value);
      } else {
        printf("error: unsupported numeric binary operator.\n");
        return FAIL;
      }
    } else if (leftNum.type == TYPE_INT && rightNum.type == TYPE_FLOAT) {
      out_value->type = TYPE_FLOAT;
      out_value->int_value = 0;
      out_value->float_value = 0.0;
      if (operator_kind == TOKEN_PLUS) {
        out_value->float_value = (double)leftNum.int_value + rightNum.float_value;
      } else if (operator_kind == TOKEN_MINUS) {
        out_value->float_value = (double)leftNum.int_value - rightNum.float_value;
      } else if (operator_kind == TOKEN_STAR) {
        out_value->float_value = (double)leftNum.int_value * rightNum.float_value;
      } else if (operator_kind == TOKEN_SLASH) {
        if (rightNum.float_value == 0) {
          printf("error: division by zero.\n");
          return FAIL;
        }
        out_value->float_value = (double)leftNum.int_value / rightNum.float_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = ((double)leftNum.int_value < rightNum.float_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = ((double)leftNum.int_value > rightNum.float_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = ((double)leftNum.int_value == rightNum.float_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = ((double)leftNum.int_value != rightNum.float_value);
      } else {
        printf("error: unsupported numeric binary operator.\n");
        return FAIL;
      }
    } else if (leftNum.type == TYPE_FLOAT && rightNum.type == TYPE_INT) {
      out_value->type = TYPE_FLOAT;
      out_value->int_value = 0;
      out_value->float_value = 0.0;
      if (operator_kind == TOKEN_PLUS) {
        out_value->float_value = leftNum.float_value + (double)rightNum.int_value;
      } else if (operator_kind == TOKEN_MINUS) {
        out_value->float_value = leftNum.float_value - (double)rightNum.int_value;
      } else if (operator_kind == TOKEN_STAR) {
        out_value->float_value = leftNum.float_value * (double)rightNum.int_value;
      } else if (operator_kind == TOKEN_SLASH) {
        if (rightNum.int_value == 0) {
          printf("error: division by zero.\n");
          return FAIL;
        }
        out_value->float_value = leftNum.float_value / (double)rightNum.int_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value < (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value > (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value == (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->float_value = 0.0;
        out_value->int_value = (leftNum.float_value != (double)rightNum.int_value);
      } else {
        printf("error: unsupported numeric binary operator.\n");
        return FAIL;
      }
    } else {
      printf("error: unsupported operand types.\n");
      return FAIL;
    }

    return SUCCESS;
  } else if (node->kind == AST_IDENTIFIER) {
    struct Symbol *symbol = lookup_symbol_visible(table, node->identifier_name);
    if (symbol) {
      *out_value = symbol->value;
      return SUCCESS;
    } else {
      printf("error: undefined identifier (%s)\n", node->identifier_name);
      return FAIL;
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
    enum TypeKind declared_variable_type = node->declared_variable_type;
    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("error: declaration must have an identifier.\n");
      return FAIL;
    }

    char *identifier = node->left->identifier_name;

    struct Value out_value = {0};
    out_value.type = declared_variable_type;
    if (declared_variable_type == TYPE_INT) {
      out_value.int_value = 0;
    } else if (declared_variable_type == TYPE_FLOAT) {
      out_value.float_value = 0.0;
    }

    struct Symbol *symbol = lookup_symbol_current(table, identifier);
    if (symbol) {
      printf("error: redeclaration of identifier %s\n", identifier);
      return FAIL;
    }

    if (node->right) {
      int ret = eval_expression(node->right, table, &out_value);
      if (ret == FAIL) {
       return FAIL;
      }
    }

    if (is_assignment_compatible(declared_variable_type, out_value.type) == 0) {
      printf("error: initializer assignment type mismatch for declared identifier %s\n", identifier);
      printf("error: expected %s but found %s type.\n",
        typekind_type_name(declared_variable_type), typekind_type_name(out_value.type));
      return FAIL;
    }

    if (declared_variable_type == TYPE_FLOAT && out_value.type == TYPE_INT) {
      out_value.type = declared_variable_type;
      out_value.float_value = (double)out_value.int_value;
      out_value.int_value = 0;
    }

    printf("declared %s with initial value", identifier);
    if (out_value.type == TYPE_INT) {
      printf(" %d\n", out_value.int_value);
    } else if (out_value.type == TYPE_FLOAT) {
      printf(" %f\n", out_value.float_value);
    } else {
      return FAIL;
    }

    store_symbol(table, declared_variable_type, identifier, &out_value);
    return SUCCESS;
  } else if (node->kind == AST_ASSIGNMENT) {
    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      printf("error: assignment to undeclared identifier %s\n", identifier);
      return FAIL;
    }

    struct Value out_value = {0};
    int ret = eval_expression(node->right, table, &out_value);
    if (ret == FAIL) {
      return FAIL;
    }

    if (is_assignment_compatible(symbol->type, out_value.type) == 0) {
      printf("error: assignment type mismatch for declared identifier %s\n", identifier);
      printf("error: expected %s but found %s type.\n",
        typekind_type_name(symbol->type), typekind_type_name(out_value.type));
      return FAIL;
    }

    if (symbol->type == TYPE_FLOAT && out_value.type == TYPE_INT) {
      out_value.type = symbol->type;
      out_value.float_value = (double)out_value.int_value;
      out_value.int_value = 0;
    }

    symbol->value = out_value;
    return SUCCESS;
  } else  if (node->kind == AST_BLOCK) {
    struct SymbolTable local_semantic_table;
    initialize_symbol_table(&local_semantic_table, table);

    for (int c=0; c < node->statement_count; c++) {
      if (node->statements[c] == NULL) {
        return FAIL;
      }

      if (eval_statement(node->statements[c], &local_semantic_table) == FAIL) {
        return FAIL;
      }
    }

    //print_symbol_table(&local_semantic_table);
    return SUCCESS;
  } else if (node->kind == AST_IF) {
    if (node->left == NULL) {
      return FAIL;
    }

    struct Value expr_value;
    int ret = eval_expression(node->left, table, &expr_value);
    if (ret == FAIL) {
      return FAIL;
    }

    if (node->then_branch == NULL) {
      return FAIL;
    }

    if ((expr_value.type == TYPE_INT && expr_value.int_value != 0)
      || (expr_value.type == TYPE_FLOAT && expr_value.float_value != 0.0)) {
      ret = eval_statement(node->then_branch, table);
      if (ret == FAIL) {
        return FAIL;
      }
    } else {
      if (node->else_branch) {
        ret = eval_statement(node->else_branch, table);
        if (ret == FAIL) {
          return FAIL;
        }
      }
    }

    return SUCCESS;
  } else if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      printf("error: print statement missing expression\n");
      return FAIL;
    }

    struct Value out_value;
    int ret = eval_expression(node->left, table, &out_value);
    if (ret == FAIL) {
      return FAIL;
    }

    if (out_value.type == TYPE_INT) {
      printf("%d\n", out_value.int_value);
    } else if (out_value.type == TYPE_FLOAT) {
      printf("%f\n", out_value.float_value);
    }
    return SUCCESS; // print statment executed successfully.
  }

  return FAIL;
}

int eval_program(struct ASTNode * node, struct SymbolTable *table) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind != AST_PROGRAM) {
    return FAIL;
  }

  for (int c=0; c < node->statement_count; c++) {
    if(eval_statement(node->statements[c], table) == FAIL) {
      return FAIL;
    }
  }
  return SUCCESS;
}

// type check over AST - NO COMPUTE
int is_assignment_compatible(enum TypeKind target_type, enum TypeKind source_type) {
  if (target_type == source_type) {
    return 1;
  }

  if ((target_type == TYPE_FLOAT) && (source_type == TYPE_INT)) {
    return 1;
  }

  if ((target_type == TYPE_INT) && (source_type == TYPE_FLOAT)) {
    return 0;
  }

  return 0;
}

int is_supported_type(enum TypeKind type) {
  switch (type) {
    case TYPE_INT:
    case TYPE_FLOAT:
      return 1;
    default:
      return 0;
  }
}

int is_supported_binary_operator(enum TokenKind operator_kind) {
  if (operator_kind == TOKEN_PLUS
    || operator_kind == TOKEN_MINUS
    || operator_kind == TOKEN_STAR
    || operator_kind == TOKEN_SLASH
    || operator_kind == TOKEN_LESS
    || operator_kind == TOKEN_GREATER
    || operator_kind == TOKEN_EQUAL_EQUAL
    || operator_kind == TOKEN_BANG_EQUAL) {

    return 1; // supported;
  }

  return 0; // not supported;
}

enum TypeKind analyze_expression_type(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    return TYPE_ERROR;
  }

  if (node->kind == AST_NUMBER) {
    return node->type_kind;
  } else if (node->kind == AST_IDENTIFIER) {

    char *identifier = node->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
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
    // int op int -> int
    // float op float -> float
    // float op int OR int op float -> promote to float

    enum TokenKind operator_kind = node->operator_kind;
    if (operator_kind == TOKEN_PLUS || operator_kind == TOKEN_MINUS ||
        operator_kind == TOKEN_STAR || operator_kind == TOKEN_SLASH) {
      if (leftType == TYPE_INT && rightType  == TYPE_INT) {
        return TYPE_INT;
      } else if (leftType == TYPE_FLOAT && rightType  == TYPE_FLOAT) {
        return TYPE_FLOAT;
      } else if (leftType == TYPE_INT && rightType  == TYPE_FLOAT) {
        return TYPE_FLOAT;
      } else if (leftType == TYPE_FLOAT && rightType  == TYPE_INT) {
        return TYPE_FLOAT;
      } else {
        printf("error: binary arithmetic expression operands have type error.\n");
        return TYPE_ERROR;
      }
    } else if (operator_kind == TOKEN_LESS || operator_kind == TOKEN_GREATER ||
        operator_kind == TOKEN_EQUAL_EQUAL || operator_kind == TOKEN_BANG_EQUAL) {
      int left_numeric = (leftType == TYPE_INT || leftType == TYPE_FLOAT);
      int right_numeric = (rightType == TYPE_INT || rightType == TYPE_FLOAT);
      if (!left_numeric || !right_numeric) {
        printf("error: comparison operands must be numeric.\n");
        return TYPE_ERROR;
      }
      return TYPE_INT;
    } else {
      printf("error: unsupported binary operator found: %s\n", token_type_name(operator_kind));
      return TYPE_ERROR;
    }
  } else if (node->kind == AST_UNARY) {
    if (node->left == NULL) {
      printf("error: unary expression missing operand\n");
      return TYPE_ERROR;
    }

    enum TypeKind operand_type = analyze_expression_type(node->left, table);
    if (is_supported_type(operand_type) == 0) {
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
    if (node->type_kind != TYPE_INT && node->type_kind != TYPE_FLOAT) {
      printf("error: number expression has invalid type\n");
      return FAIL;
    }

    return SUCCESS;
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

    // check the type of left child and set the unary node's own type_kind
    enum TypeKind child_type = analyze_expression_type(node->left, table);
    if (child_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }
    if (child_type == TYPE_INT || child_type == TYPE_FLOAT) {
      node->type_kind = child_type;
    } else {
      printf("error: currently minic only support INT/FLOAT types for operands\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }
    return SUCCESS;
  } else if (node->kind == AST_IDENTIFIER) {
    char *identifier = node->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      node->type_kind = TYPE_ERROR;
      printf("error: use of undeclared identifier %s\n", identifier);
      return FAIL;
    }
    if (symbol->type == TYPE_ERROR || symbol->type == TYPE_UNKNOWN || symbol->type == TYPE_UNUSED) {
      printf("error: identifier has not type information %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }
    node->type_kind = symbol->type;
    return SUCCESS;
  } else if (node->kind == AST_BINARY) {
    if (is_supported_binary_operator(node->operator_kind) == 0) {
      printf("error: unsupported binary operator found: %s\n", token_type_name(node->operator_kind));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_expression(node->left, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }
    ret = analyze_expression(node->right, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind leftType = analyze_expression_type(node->left, table);
    enum TypeKind rightType = analyze_expression_type(node->right, table);

    if (leftType == TYPE_ERROR || rightType == TYPE_ERROR) {
      printf("error: unsupported types found. Left has: %s and right: %s types \n",
        typekind_type_name(leftType), typekind_type_name(rightType));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TokenKind operator_kind = node->operator_kind;
    if (operator_kind == TOKEN_PLUS || operator_kind == TOKEN_MINUS ||
        operator_kind == TOKEN_STAR || operator_kind == TOKEN_SLASH) {
      if (leftType == TYPE_INT && rightType  == TYPE_INT) {
        node->type_kind = TYPE_INT;
      } else if (leftType == TYPE_FLOAT && rightType  == TYPE_FLOAT) {
        node->type_kind = TYPE_FLOAT;
      } else if (leftType == TYPE_INT && rightType  == TYPE_FLOAT) {
        node->type_kind = TYPE_FLOAT;
      } else if (leftType == TYPE_FLOAT && rightType  == TYPE_INT) {
        node->type_kind = TYPE_FLOAT;
      } else {
        printf("error: binary arithmetic expression operands have type error.\n");
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    } else if (operator_kind == TOKEN_LESS || operator_kind == TOKEN_GREATER ||
        operator_kind == TOKEN_EQUAL_EQUAL || operator_kind == TOKEN_BANG_EQUAL) {
      int left_numeric = (leftType == TYPE_INT || leftType == TYPE_FLOAT);
      int right_numeric = (rightType == TYPE_INT || rightType == TYPE_FLOAT);
      if (!left_numeric || !right_numeric) {
        printf("error: comparison operands must be numeric.\n");
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
      node->type_kind = TYPE_INT;
    } else {
      printf("error: unsupported binary operator found: %s\n", token_type_name(operator_kind));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    return SUCCESS;
  }

  printf("error: unsupported expression ASTNode: %s\n", node_type_name(node->kind));
  return FAIL;
}

int analyze_statement(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_DECLARATION) {
    enum TypeKind declared_variable_type = node->declared_variable_type;

    if (is_supported_type(declared_variable_type) == 0) {
      printf("error: currently minic found unsupported type declaration: %s type.\n",
        typekind_type_name(declared_variable_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("error: declaration must have an identifier\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol_current(table, identifier);
    if (symbol) {
      printf("error: redeclaration of identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->right) {
      int ret = analyze_expression(node->right, table);
      if (ret == FAIL) {
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }

      enum TypeKind rhs_expr_type = analyze_expression_type(node->right, table);
      if (rhs_expr_type == TYPE_ERROR) {
        printf("error: RHS expression has unsupported type: %s\n", typekind_type_name(rhs_expr_type));
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }

      if (is_assignment_compatible(declared_variable_type, rhs_expr_type) == 0) {
        printf("error: intializer assignment type mismatch for declared identifier %s\n", identifier);
        printf("error: expected %s but found %s type.\n",
          typekind_type_name(declared_variable_type), typekind_type_name(rhs_expr_type));
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    node->left->type_kind = declared_variable_type;
    struct Value value = {0};
    value.type = declared_variable_type;
    if (declared_variable_type == TYPE_INT) {
      value.int_value = 0;
    } else if (declared_variable_type == TYPE_FLOAT) {
      value.float_value = 0.0;
    }
    store_symbol(table, declared_variable_type, identifier, &value); // miniC rule: declarations without initializer get default 0 / 0.0
    node->type_kind = declared_variable_type;
    return SUCCESS;
  } else  if (node->kind == AST_ASSIGNMENT) {

    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("error: left side of an assignment statement must be an identifier\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      printf("error: assignment to undeclared identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (is_supported_type(symbol->type) == 0) {
      printf("error: unsupported type found: %s for identifier: %s.\n", typekind_type_name(symbol->type), identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->left->type_kind = symbol->type;

    if (node->right == NULL) {
      printf("error: missing assignment value for identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_expression(node->right, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind rhs_expr_type = analyze_expression_type(node->right, table);
    if (rhs_expr_type == TYPE_ERROR) {
      printf("error: RHS expression has unsupported type: %s\n", typekind_type_name(rhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (is_assignment_compatible(symbol->type, rhs_expr_type) == 0) {
      printf("error: assignment type mismatch for identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->type_kind = symbol->type;
    return SUCCESS;
  } else  if (node->kind == AST_BLOCK) {
    struct SymbolTable local_semantic_table;
    initialize_symbol_table(&local_semantic_table, table);

    for (int c=0; c < node->statement_count; c++) {
      if (node->statements[c] == NULL) {
        return FAIL;
      }

      if (analyze_statement(node->statements[c], &local_semantic_table) == FAIL) {
        return FAIL;
      }
    }

    printf("Local symbol table after analyzing statement.\n");
    print_symbol_table(&local_semantic_table);
    return SUCCESS;
  } else if (node->kind == AST_IF) {
    if (node->left == NULL) {
      printf("error: if statement requires an expression to evaluate\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_expression(node->left, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind lhs_expr_type = analyze_expression_type(node->left, table);
    if (lhs_expr_type == TYPE_ERROR) {
      printf("error: condition expression returned unsupported type: %s\n", typekind_type_name(lhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (lhs_expr_type != TYPE_INT && lhs_expr_type != TYPE_FLOAT) {
      printf("error: condition expression only supports int and float types\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->then_branch == NULL) {
      printf("error: if statement requires a then statement.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    ret = analyze_statement(node->then_branch, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->else_branch != NULL) {
      ret = analyze_statement(node->else_branch, table);
      if (ret == FAIL) {
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      printf("error: print statement requires an expression\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_expression(node->left, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind expr_type = analyze_expression_type(node->left, table);
    if (expr_type == TYPE_ERROR) {
      printf("error: print expression has unsupported type\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  }

  printf("error: unsupported statement ASTNode: %s\n", node_type_name(node->kind));
  return FAIL;
}

int analyze_program(struct ASTNode *node, struct SymbolTable *table) {
  if (node == NULL) {
    printf("error: program node is NULL\n");
    return FAIL;
  }

  if (node->kind != AST_PROGRAM) {
    printf("error: analyze_program expected AST_PROGRAM node.\n");
    return FAIL;
  }

  for (int c=0; c < node->statement_count; c++) {
    if (node->statements[c] == NULL) {
      return FAIL;
    }

    if (analyze_statement(node->statements[c], table) == FAIL) {
      return FAIL;
    }
  }

  return SUCCESS;
}

// helper parser functions
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

// datatype -> INT | FLOAT
enum TypeKind parse_datatype(struct Parser *p) {
  if (p->has_error) {
    return TYPE_ERROR;
  }

  if (is_current_token(p, TOKEN_INT)) {
    expect_token(p, TOKEN_INT);
    if ( p->has_error ) {
      return TYPE_ERROR;
    }
    return TYPE_INT;
  } else if (is_current_token(p, TOKEN_FLOAT)) {
    expect_token(p, TOKEN_FLOAT);
    if ( p->has_error ) {
      return TYPE_ERROR;
    }
    return TYPE_FLOAT;
  } else {
    p->has_error = 1;
    printf("error: unsupported data type found\n");
  }
  return TYPE_ERROR;
}

// float_literal -> digits "." digits

// integer_literal -> digits

// digits -> integer_literal | float_literal

// NUMBER -> digits
struct ASTNode * parse_number(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_NUMBER))  {
    struct Token token = p->tokens[p->pos];
    int bytesToCopy = (token.length >= BUFFER_SIZE) ? BUFFER_SIZE-1 : token.length;
    char value[BUFFER_SIZE];
    strncpy(value, token.start, bytesToCopy);
    value[bytesToCopy] = '\0';
    advance_token(p);
    // Check if the input string contains "." decimal, then it's a TYPE_FLOAT otherwise TYPE_INT
    enum TypeKind numberType = TYPE_INT;
    for (int i=0; i < bytesToCopy; i++) {
      if (value[i] == '.') {
        numberType = TYPE_FLOAT;
        break;
      }
    }
    node = createNumberNode(numberType, value);
    if (node == NULL) {
      p->has_error = 1;
      printf("error: failed to create number node\n");
    }
  }

  return node;
}

// IDENTIFIER -> x | _
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
    node = parse_number(p);
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

// factor -> unary ((STAR | SLASH) unary)*
struct ASTNode * parse_factor(struct Parser *p) {
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

// term -> factor ((PLUS | MINUS) factor)*
struct ASTNode * parse_term(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;
  left = parse_factor(p);
  if (left == NULL) {
    return NULL;
  }

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

    struct ASTNode * right = parse_factor(p);
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

// comparison -> term ((LESS | GREATER) term)*
struct ASTNode * parse_comparison(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;
  left = parse_term(p);
  if (left == NULL) {
    return NULL;
  }

  while (p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_LESS)) {
      expect_token(p, TOKEN_LESS);  // consume the token
    } else if (is_current_token(p, TOKEN_GREATER)) {
      expect_token(p, TOKEN_GREATER); // consume the token
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

// equality -> comparison ((EQUAL_EQUAL | BANG_EQUAL) comparison)*
struct ASTNode * parse_equality(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;
  left = parse_comparison(p);
  if (left == NULL) {
    return NULL;
  }

  while (p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_EQUAL_EQUAL)) {
      expect_token(p, TOKEN_EQUAL_EQUAL);  // consume the token
    } else if (is_current_token(p, TOKEN_BANG_EQUAL)) {
      expect_token(p, TOKEN_BANG_EQUAL); // consume the token
    } else {
      break;
    }

    if ( p->has_error) {
      break;
    }

    struct ASTNode * right = parse_comparison(p);
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

// expression -> equality
struct ASTNode * parse_expression(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = parse_equality(p);

  return node;
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

// declaration -> datatype IDENTIFIER SEMICOLON
//              | datatype IDENTIFIER EQUAL expression SEMICOLON
// OR
// declaration -> datatype IDENTIFIER ("=" expression)? SEMICOLON -- compact grammer version
struct ASTNode * parse_declaration(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  enum TypeKind dataType = parse_datatype(p);

  if (dataType == TYPE_ERROR || p->has_error) {
    return NULL;
  }

  struct ASTNode * identifier = parse_identifier(p);
  if ( identifier == NULL ) {
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
    free(identifier);
    if (rightValue) {
      free(rightValue);
    }
    return NULL;
  }

  struct ASTNode * node = createDeclarationNode(dataType, identifier, rightValue);
  if (node == NULL) {
    free(identifier);
    if (rightValue) {
      free(rightValue);
    }
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
     free(expr);
     return NULL;
    }

    struct ASTNode * node = createPrintNode(expr);
    if (node == NULL) {
      free(expr);
      p->has_error = 1;
    }
    return node;
  }

  return NULL;
}

// block -> '{' statement* '}'
struct ASTNode * parse_block(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // create a AST_BLOCK
  struct ASTNode * block = createBlockNode();
  if (block == NULL) {
    return NULL;
  }

  expect_token(p, TOKEN_LBRACE); // Start of a block
  if ( p->has_error ) {
    free (block);
    return NULL;
  }

  while (p->has_error == 0) {
    if (is_current_token(p, TOKEN_EOF)) {
      printf("error: expected } before of end of input\n");
      p->has_error=1;
      free (block);
      return NULL;
    }

    if (!is_current_token(p, TOKEN_RBRACE)) {
      struct ASTNode *statement = parse_statement(p);
      if (statement) {
        add_statement_to_block(block, statement);
      }
      continue;
    }

    // consume RBRACE token
    expect_token(p, TOKEN_RBRACE);
    if ( p->has_error ) {
      free (block);
      return NULL;
    }
    break;
  }

  // return whole AST_BLOCK with a list of statements
  return block;
}

// if_statement -> if ( expr ) statment else statement
//               | if ( expr ) statement
struct ASTNode * parse_if_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume IF token
  expect_token(p, TOKEN_IF);
  if ( p->has_error ) {
    return NULL;
  }

  if (!is_current_token(p, TOKEN_LPAREN)) {
    printf("error: expected '(' after IF\n");
    p->has_error = 1;
    return NULL;
  }

  // check and consume opening parenthesis
  parse_lparen(p);
  if ( p->has_error ) {
    printf("error: expected '(' after IF\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    printf("error: missing expression in IF statement\n");
    p->has_error = 1;
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if ( p->has_error ) {
    printf("error: missing closing parenthesis in IF statement\n");
    free(expr);
   return NULL;
  }

  // now parse statements. Empty statement are not allowed
  struct ASTNode * then_stmt = parse_statement(p);
  if (then_stmt == NULL) {
    printf("error: missing then statement after IF condition.\n");
    free(expr);
    p->has_error = 1;
    return NULL;
  }

  // no declaration allowed as the directy then body
  if (then_stmt->kind == AST_DECLARATION) {
    printf("error: direct declaration is not allowed as then-body; use a block\n");
    free(expr);
    free(then_stmt);
    p->has_error = 1;
    return NULL;
  }

  struct ASTNode * else_stmt = NULL;
  // now parse else statement
  if (is_current_token(p, TOKEN_ELSE)) {
    expect_token(p, TOKEN_ELSE); // consume ELSE token
    else_stmt = parse_statement(p);
    if (else_stmt == NULL) {
      printf("error: expected statement after else\n");
      p->has_error = 1;
      free(expr);
      free(then_stmt);
      return NULL;
    }
    if (else_stmt->kind == AST_DECLARATION) {
      printf("error: direct declaration is not allowed as else-body; use a block\n");
      free(expr);
      free(then_stmt);
      free(else_stmt);
      p->has_error = 1;
      return NULL;
    }
  }

  struct ASTNode * node = createIfNode(expr, then_stmt, else_stmt);
  if (node == NULL) {
    free(expr);
    free(then_stmt);
    if (else_stmt) {
      free(else_stmt);
    }
    p->has_error = 1;
  }
  return node;
}

// statement -> if_statement | block | declaration | assignment | print_statement
struct ASTNode * parse_statement(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_INT) || is_current_token(p, TOKEN_FLOAT)) {
    node = parse_declaration(p);
  } else if (is_current_token(p, TOKEN_PRINT)) {
    node = parse_print_stmt(p);
  } else if (is_current_token(p, TOKEN_LBRACE)) {
    node = parse_block(p);
  } else if (is_current_token(p, TOKEN_IF)) {
    node = parse_if_stmt(p);
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
  code="int x=4; int y=2; print((x+2)*y);";
  code="print(10/0);";
  code="int x=-5;print(x);";
  code="print(3>5);print(4>2);print(7!=7);print(8==7);";
  code="int z = x + y; int x = 1; int y = 1;";
  code="int x=40; int y=2; print(x+x/y); print(x+2*y); print(x);";
  code="int x; x=40; print(x+x/2);";
  code="print(1+2==3);print(2*3<7);print(2+3*4==14);print((2+3)*4==20);";
  code="print(1+2*3==7); print((1+2)*3==9); print(4<2+3); print(4*2==3+5);";
  code="print(1+2*3==9); print((1+2)*3==7); print(4<2+1); print(4*2==3+4);";
  code="{ int x=10; }";
  code="{int x=10; int y=3; print(x+y*2); print((x+y)*2); print(x>y); print(x==y);}";
  code="{ int x=5; int y=x;}";
  code="{int z=1; { int x=1; { int y=z; print(y); } } }";
  code="{ int x;}"; //int z = x + y; int x = 1; int y = 1;";
  code="int x=1; { int x=5; int y=x; print(x);} print(x);";
  code="{ int z=1; { int x=2; { int y=z; print(y); } } }";
  code="{ int y=5; } print(y);";
  code="int x=1; { if ( 2 > 3); int iff=2;}";
  code="int x=1; int y=2; int z = x > y;";
  code="if (0) print(1); int x=1; if (x) { x=5; print(x);} if (1) if (0) print(1);";
  code="if (0) print(123); print(1);";
  code="int x=10; if (1) { x=20; } print(x);";
  code="if (1) if (1) print(7);";
  code="int x=1; if (1) { int x=2; print(x); } print(x);";
  code="int x=5; if (x) { int y=9; print(y); } print(x);";
  code="int x=0; if (x) print(1); else { int y=2; } print(y);";
  code="if (1) print(1); else int y=2; print(y);";
  code="if (1) print(1); else { int y=2; print(y);}";
  code="if (1) int x=1; print(1); else int y=2; print(y);";
  code="if (1) { int x=1; print(x); }";
  code="if (0) { int x=1; print(x); } else { int y=2; print(y); }";
  code="if (1) if (0) print(1); else print(2);";
  code="if (0) print(1);";
  code="if (1) print(2); int x=1; int y=2; if (x > 2) print(x); else if (x < 4) { print(x); } else print(y);";
  code="int y=2; int a=2; float x=1.2; float z=3; z=y; print(z);";
  code="float x=1.2;int y=2;  x=y;";
  code="print(2+3.5); print(2 < 3.5); print(3.5 == 3); print(8/2.0); print(8.0/2);";
  code="if (0) { if (0) print(1); else { int y=0; print(4.34/y);} } else print(2 + 5);";
  code="if (0) print(1); else print(9); if (3.14) print(2); else print(0); if (-2.0) print(2); else print(0); if (0.0) print(0); else print(8);";

  printf("Input: %s\n", code);

  // Lexer or Tokenizer
  printf("\n\n--> Tokenizer PHASE: Start\n");
  int result = tokenize(code, tokens, &token_count);

  printf("--> Print tokens--\n");
  for (int i = 0; i < token_count; i++) {
    print_token(tokens[i]);
  }
  if (result == FAIL) {
    printf("--> Tokenizer PHASE: FAILED\n");
    exit(-1);
  }
  printf("--> Tokenizer PHASE: PASS\n");

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

  printf("\nStopped at: ");
  print_token(current_token(&p));

  printf("\nPrint AST Tree after PARSING PHASE:\n ");
  print_ast_tree(program, 0);

  // Check sematics of the program before we evaluate it
  printf("\n\n--> SEMANTIC CHECK PHASE: Start\n");
  printf("Initialize symbol table for Semantic check\n");
  struct SymbolTable semantic_table;
  initialize_symbol_table(&semantic_table, NULL);
  print_symbol_table(&semantic_table);
  result = analyze_program(program, &semantic_table);

  if (result == 0) {
    printf("--> SEMANTIC CHECK PHASE: FAILED\n");
    exit(-1);
  }

  printf("Global symbol table after semantic phase\n");
  print_symbol_table(&semantic_table);

  printf("\nPrint AST Tree after SEMANTIC CHECK PHASE:\n ");
  print_ast_tree(program, 0);
  printf("--> SEMANTIC CHECK PHASE: PASS\n");

  // Program is semantically okay so go ahead and evaluate it
  printf("\n\n--> EVALUATION CHECK PHASE: Start\n");
  printf("Initialize symbol table for Evaluation check\n");
  struct SymbolTable runtime_table;
  initialize_symbol_table(&runtime_table, NULL);
  print_symbol_table(&runtime_table);
  result = eval_program(program, &runtime_table);
  printf("Global symbol table after evaluation phase\n");
  print_symbol_table(&runtime_table);
  if (result == 0) {
    printf("--> EVALUATION CHECK PHASE: FAILED\n");
    exit(-1);
  }
  printf("--> EVALUATION CHECK PHASE: PASS\n");

  return 0;
}
