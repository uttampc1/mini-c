/******************************************************************************
miniC -
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SUCCESS 1
#define FAIL    0

#define TRUE    1
#define FALSE   0

#define BUFFER_SIZE 1024
#define MAX_TOKENS  1024
#define MAX_SYMBOLS 1024
#define MAX_PARAMS  32
#define SPACES 2

enum TokenKind {
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_PLUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_MINUS,
  TOKEN_PERCENT,
  TOKEN_EQUAL,
  TOKEN_LESS,
  TOKEN_GREATER,
  TOKEN_EQUAL_EQUAL,
  TOKEN_BANG_EQUAL,
  TOKEN_ANDAND,
  TOKEN_OROR,
  TOKEN_BANG,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS_EQUAL,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,
  TOKEN_BREAK,
  TOKEN_CONTINUE,
  TOKEN_DO,
  TOKEN_RETURN,
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
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
  AST_WHILE,
  AST_DO_WHILE,
  AST_FOR,
  AST_BREAK,
  AST_CONTINUE,
  AST_BLOCK,
  AST_PROGRAM,
  AST_EMPTY,
  AST_RETURN,
  AST_FUNCTION,
  AST_EXPRESSION_STATEMENT,
  AST_UNKNOWN
};

enum TypeKind {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_UNKNOWN,
  TYPE_UNUSED,
  TYPE_ERROR
};

enum EvalResult {
  EVAL_FAIL,
  EVAL_OK,
  EVAL_BREAK,
  EVAL_CONTINUE,
  EVAL_RETURN
};

struct Token {
  enum TokenKind type;
  char *start;
  int  length;
};

struct Parser {
  struct Token *tokens;
  int    token_count;
  int    pos;
  int    has_error;
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

struct FunctionSymbol {
  char *name;
  enum TypeKind return_type;
};
struct FunctionSymbolTable {
  struct FunctionSymbol function_symbols[MAX_SYMBOLS];
  int count;
};

/* Following two structures are used during semantic analysis phase */
struct ContextFrame {
  enum ASTNodeKind kind;
  enum TypeKind function_return_type;
  struct ContextFrame *prev;
};

struct SemanticContext {
  struct ContextFrame *current_ctx;
};

struct Param {
  enum TypeKind type_kind;
  char identifier_name[BUFFER_SIZE];
};

struct FunctionParam {
  struct Param params[MAX_PARAMS];
  int    count;
};

struct ASTNode {
  enum   ASTNodeKind kind;
  int    number_value;
  double float_number_value;
  char   identifier_name[BUFFER_SIZE];
  enum   TokenKind operator_kind;
  enum   TypeKind  type_kind;
  enum   TypeKind  declared_type;
  enum   TypeKind  function_type;
  char   function_name[BUFFER_SIZE];
  struct ASTNode * left;
  struct ASTNode * right;
  struct ASTNode * then_branch;
  struct ASTNode * else_branch;
  struct ASTNode * init;
  struct ASTNode * update;
  struct ASTNode * condition;
  struct ASTNode * body;
  struct ASTNode * expression;
  struct ASTNode * statements[BUFFER_SIZE];
  struct FunctionParam func_param;
  int    statement_count;
};


const char *reserved[] = { "if", "while", "do", "for", "int", "float", "return", "print", NULL };

struct ASTNode * createASTNode();
struct ASTNode * createUnaryNode(enum TokenKind type);
struct ASTNode * createNumberNode(enum TypeKind type, char * value);
struct ASTNode * createIdentifierNode(char *name);
struct ASTNode * createBinaryNode(enum TokenKind type, struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createDeclarationNode(enum TypeKind declaredType, struct ASTNode *ident, struct ASTNode *valueNode);
struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode);
struct ASTNode * createPrintNode(struct ASTNode * expr);
struct ASTNode * createIfNode(struct ASTNode * expr, struct ASTNode * then_stmt, struct ASTNode * else_stmt);
struct ASTNode * createWhileNode(struct ASTNode * expr, struct ASTNode * body_stmt);
struct ASTNode * createDoWhileNode(struct ASTNode * expr, struct ASTNode * body_stmt);
struct ASTNode * createReturnNode(struct ASTNode * expr);
struct ASTNode * createExpressionStatementNode(struct ASTNode * expr);
struct ASTNode * createFunctionDefinitionNode(enum TypeKind type, char *name, struct ASTNode * block);
struct ASTNode * createBlockNode();
struct ASTNode * createProgramNode();
struct ASTNode * createEmptyNode();

int analyze_declaration(struct ASTNode *node, struct SymbolTable *sym_table, struct FunctionSymbolTable *func_sym_table, struct SemanticContext *ctx, int global_scope);
enum TypeKind analyze_expression(struct ASTNode *node, struct SymbolTable *table);
int analyze_statement(struct ASTNode *node, struct SymbolTable *table, struct FunctionSymbolTable *func_sym_table, struct SemanticContext *ctx);
int analyze_program(struct ASTNode *node, struct SymbolTable *sym_table, struct FunctionSymbolTable *func_table, struct SemanticContext *ctx);

enum EvalResult eval_statement(struct ASTNode * root, struct SymbolTable *sym_table);

struct Symbol  * lookup_symbol_current(struct SymbolTable *sym_table, const char *name); // current scope search
struct Symbol  * lookup_symbol_visible(struct SymbolTable *sym_table, const char *name); // globalc scope search
struct ASTNode * parse_program(struct Parser *p);
struct ASTNode * parse_block(struct Parser *p);
struct ASTNode * parse_statement(struct Parser *p);
struct ASTNode * parse_expression(struct Parser *p);
struct ASTNode * parse_primary(struct Parser *p);
struct ASTNode * parse_term(struct Parser *p);
struct ASTNode * parse_empty(struct Parser *p);

int add_node_to_program(struct ASTNode * program, struct ASTNode * node);
int add_statement_to_block(struct ASTNode * node, struct ASTNode * statement);
int is_assignment_compatible(enum TypeKind target_type, enum TypeKind source_type);

enum TypeKind token_to_typekind(enum TokenKind type) {
  if (type == TOKEN_INT) {
    return TYPE_INT;
  }
  return TYPE_ERROR;
}

char *ast_node_type_name(enum ASTNodeKind kind) {
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
    case AST_WHILE:
      return "WHILE";
    case AST_DO_WHILE:
      return "DO_WHILE";
    case AST_FOR:
      return "FOR";
    case AST_BREAK:
      return "BREAK";
    case AST_CONTINUE:
      return "CONTINUE";
    case AST_PROGRAM:
      return "PROGRAM";
    case AST_EMPTY:
      return "EMPTY";
    case AST_RETURN:
      return "RETURN";
    case AST_FUNCTION:
      return "FUNCTION";
    case AST_EXPRESSION_STATEMENT:
      return "EXPRESSION_STATEMENT";
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
    case TOKEN_PERCENT:
      return "%";
    case TOKEN_EQUAL:
      return "=";
    case TOKEN_LESS:
      return "<";
    case TOKEN_LESS_EQUAL:
      return "<=";
    case TOKEN_GREATER:
      return ">";
    case TOKEN_GREATER_EQUAL:
      return ">=";
    case TOKEN_EQUAL_EQUAL:
      return "==";
    case TOKEN_BANG_EQUAL:
      return "!=";
    case TOKEN_BANG:
      return "!";
    case TOKEN_ANDAND:
      return "ANDAND";
    case TOKEN_OROR:
      return "OROR";
    case TOKEN_LPAREN:
      return "LPAREN";
    case TOKEN_RPAREN:
      return "RPAREN";
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
    case TOKEN_WHILE:
      return "WHILE";
    case TOKEN_FOR:
      return "FOR";
    case TOKEN_DO:
      return "DO";
    case TOKEN_BREAK:
      return "BREAK";
    case TOKEN_CONTINUE:
      return "CONTINUE";
    case TOKEN_RETURN:
      return "RETURN";
    case TOKEN_SEMICOLON:
      return "SEMICOLON";
    case TOKEN_COMMA:
      return "COMMA";
    case TOKEN_UNKNOWN:
      return "UNKNOWN";
    case TOKEN_EOF:
      return "EOF";
  }
  return "UNKNOWN";
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
  node->init = NULL;
  node->update = NULL;
  node->condition = NULL;
  node->body = NULL;
  node->identifier_name[0] = '\0';
  node->function_name[0] = '\0';
  node->operator_kind = TOKEN_UNKNOWN;
  node->type_kind = TYPE_UNUSED;
  node->declared_type = TYPE_UNUSED;
  for (int i=0; i < BUFFER_SIZE; i++) {
    node->statements[i] = NULL;
  }
  node->statement_count = 0;
  return node;
}

// AST_EMPTY node allocation
struct ASTNode * createEmptyNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for empty ASTNode\n");
    return NULL;
  }

  node->kind = AST_EMPTY;
  return node;
}

// AST_UNARY node allocation
struct ASTNode * createUnaryNode(enum TokenKind type) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for unary ASTNode\n");
    return NULL;
  }

  node->kind = AST_UNARY;
  node->operator_kind = type;
  return node;
}

struct ASTNode * createNumberNode(enum TypeKind type, char * value) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for number ASTNode\n");
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
    printf("AST error: couldn't allocate memory for identifier ASTNode\n");
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
    printf("AST error: couldn't allocate memory for binary ASTNode\n");
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
    printf("AST error: couldn't allocate memory for identifier ASTNode\n");
    return NULL;
  }

  node->kind = AST_DECLARATION;
  node->left = ident;
  node->right = valueNode;
  node->declared_type = declaredType;
  return node;
}

struct ASTNode * createAssignmentNode(struct ASTNode * leftNode, struct ASTNode * rightNode) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for assignment ASTNode\n");
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
    printf("AST error: couldn't allocate memory for print ASTNode\n");
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
    printf("AST error: couldn't allocate memory for block ASTNode\n");
    return NULL;
  }

  node->kind = AST_BLOCK;
  return node;
}

struct ASTNode * createBreakNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for break ASTNode\n");
    return NULL;
  }

  node->kind = AST_BREAK;
  return node;
}

struct ASTNode * createContinueNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for continue ASTNode\n");
    return NULL;
  }

  node->kind = AST_CONTINUE;
  return node;
}

struct ASTNode * createWhileNode(struct ASTNode * condition, struct ASTNode * body) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for while ASTNode\n");
    return NULL;
  }

  node->kind = AST_WHILE;
  node->condition = condition;
  node->body = body;
  return node;
}

struct ASTNode * createDoWhileNode(struct ASTNode * condition, struct ASTNode * body) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for do-while ASTNode\n");
    return NULL;
  }

  node->kind = AST_DO_WHILE;
  node->condition = condition;
  node->body = body;
  return node;
}

struct ASTNode * createForNode(struct ASTNode * init, struct ASTNode *  condition,
  struct ASTNode * update, struct ASTNode * body) {

  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for 'for' ASTNode\n");
    return NULL;
  }

  node->kind = AST_FOR;
  node->condition = condition;
  node->init = init;
  node->update = update;
  node->body = body;
  return node;
}

struct ASTNode * createIfNode(struct ASTNode * condition, struct ASTNode * then_stmt, struct ASTNode * else_stmt) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for if ASTNode\n");
    return NULL;
  }

  node->kind = AST_IF;
  node->condition = condition;
  node->then_branch = then_stmt;
  node->else_branch = else_stmt;
  return node;
}

struct ASTNode * createReturnNode(struct ASTNode * expression) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for return ASTNode\n");
    return NULL;
  }

  node->kind = AST_RETURN;
  node->expression = expression;
  return node;
}

struct ASTNode * createExpressionStatementNode(struct ASTNode * expression) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for expression stmt ASTNode\n");
    return NULL;
  }

  node->kind = AST_EXPRESSION_STATEMENT;
  node->expression = expression;
  return node;
}

struct ASTNode * createFunctionDefinitionNode(enum TypeKind type, char *name, struct ASTNode * body) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for function definition ASTNode\n");
    return NULL;
  }

  node->kind = AST_FUNCTION;
  node->function_type = type;
  strcpy(node->function_name, name);
  node->body = body;
  return node;
}

// Program node which holds a list of statements and count
struct ASTNode * createProgramNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("AST error: couldn't allocate memory for program ASTNode\n");
    return NULL;
  }

  node->kind = AST_PROGRAM;
  return node;
}

void freeASTNode(struct ASTNode * node) {
  if (node == NULL) {
    return;
  }

  if (node->left != NULL) {
    freeASTNode(node->left);
  }
  if (node->right != NULL) {
    freeASTNode(node->right);
  }
  if (node->condition != NULL) {
    freeASTNode(node->condition);
  }
  if (node->then_branch != NULL) {
    freeASTNode(node->then_branch);
  }
  if (node->else_branch != NULL) {
    freeASTNode(node->else_branch);
  }
  if (node->body != NULL) {
    freeASTNode(node->body);
  }

  if (node->statement_count) {
    for(int i=0; i < node->statement_count; i++) {
      if (node->statements[i] != NULL) {
        freeASTNode(node->statements[i]);
      }
    }
  }

  free(node);
}

void freeFunctionSymbolTableContents(struct FunctionSymbolTable *table) {
  if (table == NULL) {
    return;
  }

  for (int i=0; i < table->count; i++) {
    if (table->function_symbols[i].name != NULL) {
      free(table->function_symbols[i].name);
      table->function_symbols[i].name = NULL;
    }
  }

  table->count = 0;
}

void freeSymbolTableContents(struct SymbolTable *table) {
  if (table == NULL) {
    return;
  }

  for (int i=0; i < table->count; i++) {
    if (table->symbols[i].name != NULL) {
      free(table->symbols[i].name);
      table->symbols[i].name = NULL;
    }
  }

  table->count = 0;
}

/********************************************
 * logging/printing Tokens, AST_tree
 ********************************************/
void print_token(struct Token t) {
  if (t.type == TOKEN_EOF) {
    printf("EOF\n");
  } else {
    printf("%s: %.*s\n", token_type_name(t.type), t.length, t.start);
  }
}

void print_function_symbol_table(struct FunctionSymbolTable *table) {
  if (table == NULL) {
    return;
  }

  printf("--- print Function Symbol Table ---\n");
  if (table->count <= 0) {
    printf("no symbols found\n");
    return;
  }

  for (int i = 0; i < table->count; i++) {
    printf("[%d] name=%s, type=%s\n",
      i, table->function_symbols[i].name,
      typekind_type_name(table->function_symbols[i].return_type));
  }

  return;
}

void print_symbol_table(struct SymbolTable *table) {
  struct SymbolTable * t = table;
  if (t == NULL) {
    return;
  }

  printf("--- print Variable Symbol Table ---\n");
  if (t->count <= 0) {
    printf("no symbols found\n");
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
    printf("%s %s\n", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    for (int c=0; c < t->statement_count; c++) {
      print_ast_tree(t->statements[c], depth+1);
    }
    return;
  } else if (t->kind == AST_EMPTY) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_FUNCTION) {
    printf("%*s %s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), t->function_name, typekind_type_name(t->type_kind));
    struct FunctionParam func_param = t->func_param;
    if (func_param.count > 0) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "PARAMETERS");
      for (int i=0; i < func_param.count; i++) {
         printf("%*s %s %s\n", SPACES*(depth+2), " ", typekind_type_name(func_param.params[i].type_kind), t->func_param.params[i].identifier_name);
      }
    }

    if (t->body != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "BODY");
      print_ast_tree(t->body, depth+2);
    }
    
  } else if (t->kind == AST_RETURN) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->expression != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "EXPRESSION");
      print_ast_tree(t->expression, depth+2);
    }
  } else if (t->kind == AST_EXPRESSION_STATEMENT) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->expression != NULL) {
      print_ast_tree(t->expression, depth+1);
    }
  } else if (t->kind == AST_DECLARATION) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_BLOCK) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    for (int c=0; c < t->statement_count; c++) {
      print_ast_tree(t->statements[c], depth+1);
    }
  } else if (t->kind == AST_IF) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->condition != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "CONDITION");
      print_ast_tree(t->condition, depth+2);
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
  } else if (t->kind == AST_WHILE) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->condition != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "CONDITION");
      print_ast_tree(t->condition, depth+2);
    }
    if (t->body != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "BODY");
      print_ast_tree(t->body, depth+2);
    }
  } else if (t->kind == AST_DO_WHILE) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->body != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "BODY");
      print_ast_tree(t->body, depth+2);
    }
    if (t->condition != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "CONDITION");
      print_ast_tree(t->condition, depth+2);
    }
  } else if (t->kind == AST_FOR) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    if (t->init != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "INIT");
      print_ast_tree(t->init, depth+2);
    }
    if (t->condition != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "CONDITION");
      print_ast_tree(t->condition, depth+2);
    }
    if (t->update != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "UPDATE");
      print_ast_tree(t->update, depth+2);
    }
    if (t->body != NULL) {
      printf("%*s %s \n", SPACES*(depth+1), " ", "BODY");
      print_ast_tree(t->body, depth+2);
    }
  } else if (t->kind == AST_BREAK) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_CONTINUE) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_PRINT) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_ASSIGNMENT) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_NUMBER) {
    if (t->type_kind == TYPE_INT) {
      printf("%*s %s %d %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), t->number_value, typekind_type_name(t->type_kind));
    } else  if (t->type_kind == TYPE_FLOAT) {
      printf("%*s %s %f %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), t->float_number_value, typekind_type_name(t->type_kind));
    }
  } else if (t->kind == AST_IDENTIFIER) {
    printf("%*s %s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), t->identifier_name, typekind_type_name(t->type_kind));
  } else if (t->kind == AST_BINARY) {
    printf("%*s %s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), token_type_name(t->operator_kind), typekind_type_name(t->type_kind));
  } else if (t->kind == AST_UNARY) {
    printf("%*s %s %s\n", SPACES*depth, " ", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
  } else {
    printf("%*s UNKNOWN AST NODE\n", SPACES*depth, " ");
  }

  print_ast_tree(t->left, depth+1);
  print_ast_tree(t->right, depth+1);
  print_ast_tree(t->else_branch, depth+1);

  return;
}

/********************************************
 * helper functions
 ********************************************/
void print_debug_output(struct ASTNode * program, struct SymbolTable *sym_table, struct FunctionSymbolTable *func_table) {
  printf("\n*********************** DEBUG output ************** \n");
  printf("Print AST Tree, function and variable symbol table after SEMANTIC CHECK PHASE:\n ");
  print_ast_tree(program, 0);
  printf("\n");
  print_symbol_table(sym_table);
  printf("\n");
  print_function_symbol_table(func_table);
  printf("*********************** DEBUG output ************** \n\n");
  return;
}

int statement_guarantees_return(struct ASTNode * node) {
  if (node == NULL) {
    return FALSE;
  }

  if (node->kind == AST_RETURN) {
    return TRUE;
  }

  if (node->kind == AST_BLOCK) {
    for (int c=0; c < node->statement_count; c++) {
      if (node->statements[c] != NULL) {
        int stmt_return = statement_guarantees_return(node->statements[c]);
        if (stmt_return == TRUE) {
          return TRUE;
        }
      }
    }

    return FALSE;
  }

  if (node->kind == AST_IF) {
    int then_returns = FALSE;
    int else_returns = FALSE;
    if (node->then_branch != NULL) {
      then_returns = statement_guarantees_return(node->then_branch);
    }
    if (node->else_branch != NULL) {
      else_returns = statement_guarantees_return(node->else_branch);
    }

    return then_returns && else_returns;
  }

  return FALSE;
}

int is_assignment_compatible(enum TypeKind target_type, enum TypeKind source_type) {
  if (target_type == source_type) {
    return TRUE;
  }

  if ((target_type == TYPE_FLOAT) && (source_type == TYPE_INT)) {
    return TRUE;
  }

  // Any other combination is incompatible
  return FALSE;
}

int is_supported_type(enum TypeKind type) {
  switch (type) {
    case TYPE_INT:
    case TYPE_FLOAT:
      return TRUE;
    default:
      return FALSE;
  }
}

int is_supported_binary_operator(enum TokenKind operator_kind) {
  if (operator_kind == TOKEN_PLUS
    || operator_kind == TOKEN_MINUS
    || operator_kind == TOKEN_STAR
    || operator_kind == TOKEN_SLASH
    || operator_kind == TOKEN_PERCENT
    || operator_kind == TOKEN_LESS
    || operator_kind == TOKEN_LESS_EQUAL
    || operator_kind == TOKEN_GREATER
    || operator_kind == TOKEN_GREATER_EQUAL
    || operator_kind == TOKEN_EQUAL_EQUAL
    || operator_kind == TOKEN_BANG_EQUAL
    || operator_kind == TOKEN_ANDAND
    || operator_kind == TOKEN_OROR) {

    return TRUE; // supported;
  }

  return FALSE; // not supported;
}

struct ContextFrame * find_enclosing_context(struct SemanticContext *ctx, enum ASTNodeKind target_kind) {
  if (ctx == NULL || ctx->current_ctx == NULL) {
    return NULL;
  }

  struct ContextFrame * current_ctx = ctx->current_ctx;

  while(current_ctx != NULL) {
    if (current_ctx->kind == target_kind) {
      break;
    }

    current_ctx = current_ctx->prev;
  }

  return current_ctx;
}

int has_enclosing_kind(struct ContextFrame *context_frame, enum ASTNodeKind target_kind) {
  if (context_frame == NULL) {
    return FALSE;
  }

  if (context_frame->kind != target_kind) {
    return has_enclosing_kind(context_frame->prev, target_kind);
  }

  return TRUE;
}

int is_inside_loop(struct SemanticContext *ctx) {
  if (ctx == NULL || ctx->current_ctx == NULL) {
    return FALSE;
  }

  return (
    has_enclosing_kind(ctx->current_ctx, AST_FOR) ||
    has_enclosing_kind(ctx->current_ctx, AST_DO_WHILE) ||
    has_enclosing_kind(ctx->current_ctx, AST_WHILE)
   );
}

int is_inside_function(struct SemanticContext *ctx) {
  if (ctx == NULL || ctx->current_ctx == NULL) {
    return FALSE;
  }

  return has_enclosing_kind(ctx->current_ctx, AST_FUNCTION);
}

int add_node_to_program(struct ASTNode * program_node, struct ASTNode * node) {
  struct ASTNode * program = program_node;

  // we can add few checks
  // program != NULL
  // program->kind == AST_PROGRAM
  // statement != NULL
  if (program && (program->kind == AST_PROGRAM) && (node != NULL) ) {
    int count = program->statement_count;

    if (count < BUFFER_SIZE) {
      program->statements[count] = node;
      program->statement_count = count + 1;
    } else {
      printf("error: we are running out of space to add additional nodes in a program\n");
      return FAIL;
    }
  }
  return SUCCESS;
}

int add_statement_to_block(struct ASTNode * node, struct ASTNode * statement) {
  struct ASTNode * block = node;

  // we can add few checks
  // program != NULL
  // program->kind == AST_PROGRAM
  // statement != NULL
  if ( block && (block->kind == AST_BLOCK) && (statement != NULL) ) {
    int count = block->statement_count;

    if (count < BUFFER_SIZE) {
     block->statements[count] = statement;
     block->statement_count = count + 1;
    } else {
      printf("error: we are running out of space to add additional statements in block\n");
      return FAIL;
    }
  }
  return SUCCESS;
}

void initialize_function_symbol_table(struct FunctionSymbolTable *table) {
  if (table == NULL) {
    return;
  }

  for (int i=0; i < MAX_SYMBOLS; i++) {
    table->function_symbols[i].name = NULL;
  }

  table->count = 0;
}

// return function symbol entry if it exists
struct FunctionSymbol * lookup_function_symbol(struct FunctionSymbolTable *table, char *name) {
  if (table == NULL) {
    return NULL;
  }

  if (name == NULL) {
    printf("semantic error: input function name to search is empty\n");
    return NULL;
  }

  for (int i = 0; i < table->count; i++) {
    struct FunctionSymbol *symbol = &table->function_symbols[i];
    if (symbol->name != NULL && strcmp(symbol->name, name) == 0) {
      return symbol;
    }
  }

  return NULL;
}

int store_function_symbol(struct FunctionSymbolTable *table, enum TypeKind return_type, const char *name) {
  if (table == NULL) {
    printf("semantic error: function symbol table is not allocated\n");
    return FAIL;
  }

  if (name == NULL) {
    printf("semantic error: input function name is empty\n");
    return FAIL;
  }

  struct FunctionSymbol *symbol = NULL;
  for (int i = 0; i < table->count; i++) {
    symbol = &table->function_symbols[i];
    if (symbol->name != NULL && strcmp(symbol->name, name) == 0) {
      printf("semantic error: function '%s' is already defined\n", name);
      return FAIL;
    }
  }

  if (table->count >= MAX_SYMBOLS) {
    printf("semantic error: function symbol table is full, no space left.\n");
    return FAIL;
  }

  size_t symbol_len = strlen(name);
  char *new_symbol = (char *)malloc(symbol_len+1);
  if (new_symbol == NULL) {
    printf("semantic error: couldn't allocate memory for new function '%s' entry\n", name);
    return FAIL;
  }

  for (int i=0; i < symbol_len+1; i++) {
    new_symbol[i] = '\0';
  }

  strcpy(new_symbol, name);

  table->function_symbols[table->count].return_type = return_type;
  table->function_symbols[table->count].name = new_symbol;
  table->count++;

  return SUCCESS;
}

// Variable Symbol table
void initialize_symbol_table(struct SymbolTable *table, struct SymbolTable *parent) {
  table->parent = parent;
  for (int i=0; i < MAX_SYMBOLS; i++) {
    table->symbols[i].name = NULL;
  }
  table->count = 0;
}

// return symbol(variable) if it exists in current scope
struct Symbol * lookup_symbol_current(struct SymbolTable *table, const char *name) {
  if (table == NULL) {
    return NULL;
  }

  for (int i = 0; i < table->count; i++) {
    struct Symbol *symbol = &table->symbols[i];
    if (symbol->name != NULL && strcmp(symbol->name, name) == 0) {
      return symbol;
    }
  }

  return NULL;
}

// if current scope doesn't have, check its parent, until
// either it is found or parent is NULL.
struct Symbol * lookup_symbol_visible(struct SymbolTable *table, const char *name) {
  if (table == NULL) {
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

// add symbol(variable) if symbol doesn't exists in the table
// update value if symbol exists in the table
void store_symbol(struct SymbolTable *table, enum TypeKind type, const char *name, struct Value *value) {
  //printf("Add [%s] symbol to the table\n", name);
  if (table == NULL) {
    printf("Symbol table error: symbol table is not allocated\n");
    return;
  }

  struct Symbol *symbol = NULL;
  for (int i = 0; i < table->count; i++) {
    symbol = &table->symbols[i];
    if (symbol->name != NULL && strcmp(symbol->name, name) == 0) {
      symbol->value = *value;
      return;
    }
  }

  if (table->count >= MAX_SYMBOLS) {
    printf("Symbol table error: symbol table is full, no space left.\n");
    return;
  }

  size_t symbol_len = strlen(name);
  char *new_symbol = (char *)malloc(symbol_len+1);
  if (new_symbol == NULL) {
    printf("Symbol table error: couldn't allocate memory for new symbol\n");
    return;
  }

  for (int i=0; i < symbol_len+1; i++) {
    new_symbol[i] = '\0';
  }
  strcpy(new_symbol, name);

  table->symbols[table->count].type = type;
  table->symbols[table->count].name = new_symbol;
  table->symbols[table->count].value = *value;
  table->count++;

  return;
}

// lexer
int add_token(struct Token tokens[], int *token_count, enum TokenKind type, char *start, int length) {
  struct Token t;
  t.type = type;
  t.start = start;
  t.length = length;
  if (*token_count < MAX_TOKENS) {
    tokens[*token_count] = t;
    *token_count = *token_count + 1;
    return SUCCESS;
  } else {
    printf("tokenizer error: no more tokens will be accepted. OOM\n");
    return FAIL;
  }
}

int is_alpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

int is_percent(char c) {
  return (c == '%');
}

int is_and(char c) {
  return (c == '&');
}

int is_or(char c) {
  return (c == '|');
}

int is_bang(char c) {
  return (c == '!');
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

int is_comma(char c) {
  return (c == ',');
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
  int ret=0;
  while(code[idx] != '\0') {
    ret = SUCCESS;
    if (is_space(code[idx])) {
      skip_spaces(code, &idx);
    } else if (is_digit(code[idx])) {
      int start = idx;
      idx = scan_number(code, idx);
      ret = add_token(tokens, token_count, TOKEN_NUMBER, code + start, idx - start);
    } else if (is_plus(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_PLUS, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_minus(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_MINUS, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_percent(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_PERCENT, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_semicolon(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_SEMICOLON, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_comma(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_COMMA, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_star(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_STAR, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_slash(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_SLASH, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_equal(code[idx])) {
      if (code[idx+1] != '\0' && is_equal(code[idx+1])) {
        ret = add_token(tokens, token_count, TOKEN_EQUAL_EQUAL, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        ret = add_token(tokens, token_count, TOKEN_EQUAL, code + idx, 1);
        if (ret == SUCCESS) {
          idx++;
        }
      }
    } else if (is_less(code[idx])) {
      if (code[idx+1] != '\0' && is_equal(code[idx+1])) {
        ret = add_token(tokens, token_count, TOKEN_LESS_EQUAL, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        ret = add_token(tokens, token_count, TOKEN_LESS, code + idx, 1);
        if (ret == SUCCESS) {
          idx++;
        }
      }
    } else if (is_greater(code[idx])) {
      if (code[idx+1] != '\0' && is_equal(code[idx+1])) {
        ret = add_token(tokens, token_count, TOKEN_GREATER_EQUAL, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        ret = add_token(tokens, token_count, TOKEN_GREATER, code + idx, 1);
        if (ret == SUCCESS) {
          idx++;
        }
      }
    } else if (is_bang(code[idx])) {
      if (code[idx+1] != '\0' && is_equal(code[idx + 1])) {
        ret = add_token(tokens, token_count, TOKEN_BANG_EQUAL, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        ret = add_token(tokens, token_count, TOKEN_BANG, code + idx, 1);
        if (ret == SUCCESS) {
          idx++;
        }
      }
    } else if (is_and(code[idx])) {
      if (code[idx+1] != '\0' && is_and(code[idx + 1])) {
        ret = add_token(tokens, token_count, TOKEN_ANDAND, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        printf("tokenizer error: unexpected character '&' after '&'\n");
        return FAIL;
      }
    } else if (is_or(code[idx])) {
      if (code[idx+1] != '\0' && is_or(code[idx + 1])) {
        ret = add_token(tokens, token_count, TOKEN_OROR, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        printf("tokenizer error: unexpected character '|' after '|'\n");
        return FAIL;
      }
    } else if (is_lparen(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_LPAREN, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_rparen(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_RPAREN, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_lbrace(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_LBRACE, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_rbrace(code[idx])) {
      ret = add_token(tokens, token_count, TOKEN_RBRACE, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    } else if (is_alpha(code[idx])) {
      int start = idx;
      idx = scan_identifier(code, idx);
      // keywords
      int len = idx - start;
      if ((len == 3) && strncmp(code+start, "int", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_INT, code + start, len);
      } else if ((len == 5) && strncmp(code+start, "float", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_FLOAT, code + start, len);
      } else if ((len == 2) && strncmp(code+start, "if", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_IF, code + start, len);
      } else if ((len == 4) && strncmp(code+start, "else", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_ELSE, code + start, len);
      } else if ((len == 5) && strncmp(code+start, "while", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_WHILE, code + start, len);
      } else if ((len == 3) && strncmp(code+start, "for", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_FOR, code + start, len);
      } else if ((len == 2) && strncmp(code+start, "do", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_DO, code + start, len);
      } else if ((len == 5) && strncmp(code+start, "break", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_BREAK, code + start, len);
      } else if ((len == 8) && strncmp(code+start, "continue", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_CONTINUE, code + start, len);
      } else if ((len == 6) && strncmp(code+start, "return", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_RETURN, code + start, len);
      } else if ((len == 5) && strncmp(code+start, "print", len) == 0) {
        // internal print() function/keyword
        ret = add_token(tokens, token_count, TOKEN_PRINT, code + start, len);
      } else {
        // identifiers
        ret = add_token(tokens, token_count, TOKEN_IDENTIFIER, code + start, len);
      }
    } else {
      ret = add_token(tokens, token_count, TOKEN_UNKNOWN, code + idx, 1);
      if (ret == SUCCESS) {
        idx++;
      }
    }

    if (ret == FAIL) {
      return FAIL;
    }
  }

  ret = add_token(tokens, token_count, TOKEN_EOF, code + idx, 0);
  if (ret == FAIL) {
    return FAIL;
  }
  return SUCCESS;
}

// Parser
int is_nth_token(struct Parser *p, int n, enum TokenKind type) {
  struct Token t = p->tokens[p->pos+n];
  if (t.type == type) {
    return TRUE;
  }
  return FALSE;
}

int is_next_token(struct Parser *p, enum TokenKind type) {
  struct Token t = p->tokens[p->pos+1];
  if (t.type == type) {
    return TRUE;
  }
  return FALSE;
}

int is_current_token(struct Parser *p, enum TokenKind type) {
  struct Token t = p->tokens[p->pos];
  if (t.type == type) {
    return TRUE;
  }
  return FALSE;
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
    printf("parse error: expected %s token, found %s token instead\n", token_type_name(type), token_type_name(t.type));
  }
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
      out_value->float_value = 0.0;
    } else if (node->type_kind == TYPE_FLOAT) {
      out_value->float_value = node->float_number_value;
      out_value->int_value = 0;
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
        out_value->float_value = 0.0;
      } else if (result.type == TYPE_FLOAT) {
        out_value->float_value = -(result.float_value);
        out_value->int_value = 0;
      } else {
        return FAIL;
      }
    } else if (node->operator_kind == TOKEN_PLUS) {
      if (result.type == TYPE_INT) {
        out_value->int_value = result.int_value;
        out_value->float_value = 0.0;
      } else if (result.type == TYPE_FLOAT) {
        out_value->float_value = result.float_value;
        out_value->int_value = 0;
      } else {
        return FAIL;
      }
    } else if (node->operator_kind == TOKEN_BANG) {
      out_value->type = TYPE_INT;
      out_value->float_value = 0.0;
      if (result.type == TYPE_INT) {
        if (result.int_value == 0) {
          out_value->int_value = 1;
        } else {
          out_value->int_value = 0;
        }
      } else if (result.type == TYPE_FLOAT) {
        if (result.float_value == 0.0) {
          out_value->int_value = 1;
        } else {
          out_value->int_value = 0;
        }
      } else {
        return FAIL;
      }
    } else {
      printf("runtime error: unsupported unary operator\n");
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
      || operator_kind == TOKEN_EQUAL_EQUAL || operator_kind == TOKEN_BANG_EQUAL
      || operator_kind == TOKEN_LESS_EQUAL  || operator_kind == TOKEN_GREATER_EQUAL
      || operator_kind == TOKEN_ANDAND      || operator_kind == TOKEN_OROR) {
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
          printf("runtime error: division by zero.\n");
          return FAIL;
        }
        out_value->int_value = leftNum.int_value / rightNum.int_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->int_value = (leftNum.int_value < rightNum.int_value);
      } else if (operator_kind == TOKEN_LESS_EQUAL) {
        out_value->int_value = (leftNum.int_value <= rightNum.int_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->int_value = (leftNum.int_value > rightNum.int_value);
      } else if (operator_kind == TOKEN_GREATER_EQUAL) {
        out_value->int_value = (leftNum.int_value >= rightNum.int_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->int_value = (leftNum.int_value == rightNum.int_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->int_value = (leftNum.int_value != rightNum.int_value);
      } else if (operator_kind == TOKEN_ANDAND) {
        int ret = (leftNum.int_value != 0);
        if (ret) {
          ret = (rightNum.int_value != 0);
        }

        out_value->int_value = ret;

      } else if (operator_kind == TOKEN_OROR) {

        int ret = (leftNum.int_value != 0);
        if (ret == 0) {
          ret = (rightNum.int_value != 0);
        }
        out_value->int_value = ret;

      } else if (operator_kind == TOKEN_PERCENT) {
        if (rightNum.int_value == 0) {
          printf("runtime error: modulo by zero.\n");
          return FAIL;
        }
        out_value->int_value = leftNum.int_value % rightNum.int_value;
      } else {
        printf("runtime error: unsupported integer binary operator.\n");
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
          printf("runtime error: division by zero.\n");
          return FAIL;
        }
        out_value->float_value = leftNum.float_value / rightNum.float_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value < rightNum.float_value);
      } else if (operator_kind == TOKEN_LESS_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value <= rightNum.float_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value > rightNum.float_value);
      } else if (operator_kind == TOKEN_GREATER_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value >= rightNum.float_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value == rightNum.float_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value != rightNum.float_value);
      } else if (operator_kind == TOKEN_ANDAND) {
        out_value->type = TYPE_INT;
        int ret = (leftNum.float_value != 0.0);
        if (ret) {
          ret = (rightNum.float_value != 0.0);
        }

        out_value->int_value = ret;

      } else if (operator_kind == TOKEN_OROR) {
        out_value->type = TYPE_INT;

        int ret = (leftNum.float_value != 0.0);
        if (ret == 0) {
          ret = (rightNum.float_value != 0.0);
        }
        out_value->int_value = ret;

      } else {
        printf("runtime error: unsupported numeric binary operator.\n");
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
          printf("runtime error: division by zero.\n");
          return FAIL;
        }
        out_value->float_value = (double)leftNum.int_value / rightNum.float_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->type = TYPE_INT;
        out_value->int_value = ((double)leftNum.int_value < rightNum.float_value);
      } else if (operator_kind == TOKEN_LESS_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = ((double)leftNum.int_value <= rightNum.float_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->type = TYPE_INT;
        out_value->int_value = ((double)leftNum.int_value > rightNum.float_value);
      } else if (operator_kind == TOKEN_GREATER_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = ((double)leftNum.int_value >= rightNum.float_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = ((double)leftNum.int_value == rightNum.float_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = ((double)leftNum.int_value != rightNum.float_value);
      } else if (operator_kind == TOKEN_ANDAND) {
        out_value->type = TYPE_INT;
        int ret = (leftNum.int_value != 0);
        if (ret) {
          ret = (rightNum.float_value != 0.0);
        }

        out_value->int_value = ret;

      } else if (operator_kind == TOKEN_OROR) {
        out_value->type = TYPE_INT;
        int ret = (leftNum.int_value != 0);
        if (ret == 0) {
          ret = (rightNum.float_value != 0.0);
        }
        out_value->int_value = ret;

      } else {
        printf("runtime error: unsupported numeric binary operator.\n");
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
          printf("runtime error: division by zero.\n");
          return FAIL;
        }
        out_value->float_value = leftNum.float_value / (double)rightNum.int_value;
      } else if (operator_kind == TOKEN_LESS) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value < (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_LESS_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value <= (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_GREATER) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value > (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_GREATER_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value >= (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_EQUAL_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value == (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_BANG_EQUAL) {
        out_value->type = TYPE_INT;
        out_value->int_value = (leftNum.float_value != (double)rightNum.int_value);
      } else if (operator_kind == TOKEN_ANDAND) {
        out_value->type = TYPE_INT;
        int ret = (leftNum.float_value != 0.0);
        if (ret) {
          ret = (rightNum.int_value != 0);
        }

        out_value->int_value = ret;

      } else if (operator_kind == TOKEN_OROR) {
        out_value->type = TYPE_INT;
        int ret = (leftNum.float_value != 0.0);
        if (ret == 0) {
          ret = (rightNum.int_value != 0);
        }
        out_value->int_value = ret;

      } else {
        printf("runtime error: unsupported numeric binary operator.\n");
        return FAIL;
      }
    } else {
      printf("runtime error: unsupported operand types.\n");
      return FAIL;
    }

    return SUCCESS;
  } else if (node->kind == AST_IDENTIFIER) {
    struct Symbol *symbol = lookup_symbol_visible(table, node->identifier_name);
    if (symbol) {
      *out_value = symbol->value;
      return SUCCESS;
    } else {
      printf("runtime error: undefined identifier (%s)\n", node->identifier_name);
      return FAIL;
    }
  } else if (node->kind == AST_ASSIGNMENT) {
    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      printf("runtime error: assignment targeted to undeclared identifier '%s'\n", identifier);
      return FAIL;
    }

    struct Value expr_value = {0};
    int ret = eval_expression(node->right, table, &expr_value);
    if (ret == FAIL) {
      return FAIL;
    }

    if (symbol->type == TYPE_FLOAT && expr_value.type == TYPE_INT) {
      expr_value.type = symbol->type;
      expr_value.float_value = (double)expr_value.int_value;
      expr_value.int_value = 0;
    } 

    symbol->value = expr_value;
    *out_value = expr_value;
    return SUCCESS;
  } else {
    printf("runtime error: Invalid AST_NODE\n");
  }

  return FAIL;
}

enum EvalResult eval_statement(struct ASTNode * root, struct SymbolTable *table) {
  struct ASTNode * node = root;
  if (node == NULL) {
    return EVAL_FAIL;
  }

  if (node->kind == AST_EMPTY) {
    return EVAL_OK;
  } else if (node->kind == AST_DECLARATION) {
    enum TypeKind declared_type = node->declared_type;
    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("runtime error: declaration must have an identifier.\n");
      return EVAL_FAIL;
    }

    char *identifier = node->left->identifier_name;

    struct Value out_value = {0};
    out_value.type = declared_type;
    if (declared_type == TYPE_INT) {
      out_value.int_value = 0;
    } else if (declared_type == TYPE_FLOAT) {
      out_value.float_value = 0.0;
    }

    struct Symbol *symbol = lookup_symbol_current(table, identifier);
    if (symbol) {
      printf("runtime error: redeclaration of identifier %s\n", identifier);
      return EVAL_FAIL;
    }

    if (node->right) {
      int ret = eval_expression(node->right, table, &out_value);
      if (ret == FAIL) {
       return EVAL_FAIL;
      }
    }

    if (is_assignment_compatible(declared_type, out_value.type) == FALSE) {
      printf("runtime error: initializer assignment type mismatch for declared identifier %s\n", identifier);
      printf("runtime error: expected %s but found %s type.\n",
        typekind_type_name(declared_type), typekind_type_name(out_value.type));
      return EVAL_FAIL;
    }

    if (declared_type == TYPE_FLOAT && out_value.type == TYPE_INT) {
      out_value.type = declared_type;
      out_value.float_value = (double)out_value.int_value;
      out_value.int_value = 0;
    }

    //printf("runtime-msg : declared %s with initial value", identifier);
    /*
    if (out_value.type == TYPE_INT) {
      printf("%d\n", out_value.int_value);
    } else if (out_value.type == TYPE_FLOAT) {
      printf("%f\n", out_value.float_value);
    } else {
      return EVAL_FAIL;
    }
    */

    // commit to the runtime symbol map
    store_symbol(table, declared_type, identifier, &out_value);
    return EVAL_OK;
  } else if (node->kind == AST_BLOCK) {
    struct SymbolTable local_runtime_table;
    initialize_symbol_table(&local_runtime_table, table);

    for (int c=0; c < node->statement_count; c++) {
      if (node->statements[c] == NULL) {
        freeSymbolTableContents(&local_runtime_table);
        return EVAL_FAIL;
      }

      enum EvalResult eval_ret = eval_statement(node->statements[c], &local_runtime_table);
      if (eval_ret == EVAL_FAIL) {
        freeSymbolTableContents(&local_runtime_table);
        return EVAL_FAIL;
      } else if (eval_ret == EVAL_BREAK) {
        freeSymbolTableContents(&local_runtime_table);
        return EVAL_BREAK;
      } else if (eval_ret == EVAL_CONTINUE) {
        freeSymbolTableContents(&local_runtime_table);
        return EVAL_CONTINUE;
      }
    }

    print_symbol_table(&local_runtime_table);
    freeSymbolTableContents(&local_runtime_table); // CLEANUP on successful block exiy
    return EVAL_OK;
  } else if (node->kind == AST_IF) {
    if (node->condition == NULL) {
      return EVAL_FAIL;
    }

    struct Value expr_value;
    int ret = eval_expression(node->condition, table, &expr_value);
    if (ret == FAIL) {
      return EVAL_FAIL;
    }

    if (node->then_branch == NULL) {
      return EVAL_FAIL;
    }

    if ((expr_value.type == TYPE_INT && expr_value.int_value != 0)
      || (expr_value.type == TYPE_FLOAT && expr_value.float_value != 0.0)) {
      enum EvalResult eval_ret = eval_statement(node->then_branch, table);
      if (eval_ret != EVAL_OK) {
        return eval_ret;
      }
    } else {
      if (node->else_branch) {
        enum EvalResult eval_ret = eval_statement(node->else_branch, table);
        if (eval_ret != EVAL_OK) {
          return eval_ret;
        }
      }
    }

    return EVAL_OK;
  } else if (node->kind == AST_WHILE) {
    if (node->condition == NULL) {
      return EVAL_FAIL;
    }

    if (node->body == NULL) {
      return EVAL_FAIL;
    }

    enum EvalResult eval_ret = eval_statement(node->body, table);
    if (eval_ret == EVAL_FAIL) {
      return EVAL_FAIL;
    }

    while(1) {
      struct Value expr_value;
      int ret = eval_expression(node->condition, table, &expr_value);
      if (ret == FAIL) {
        return EVAL_FAIL;
      }

      if (expr_value.type != TYPE_INT && expr_value.type != TYPE_FLOAT) {
        printf("runtime error: condition type must be numeric\n");
        return EVAL_FAIL;
      }

      if ((expr_value.type == TYPE_INT && expr_value.int_value == 0)
        || (expr_value.type == TYPE_FLOAT && expr_value.float_value == 0.0)) {
        break;
      }

      enum EvalResult eval_ret = eval_statement(node->body, table);
      if (eval_ret == EVAL_FAIL) {
        return EVAL_FAIL;
      } else if (eval_ret == EVAL_BREAK) {
        break;
      } else if (eval_ret == EVAL_CONTINUE) {
        continue;
      }
    }
    return EVAL_OK;
  } else if (node->kind == AST_DO_WHILE) {
    if (node->body == NULL) {
      return EVAL_FAIL;
    }

    if (node->condition == NULL) {
      return EVAL_FAIL;
    }

    while(1) {
      enum EvalResult eval_ret = eval_statement(node->body, table);
      if (eval_ret == EVAL_FAIL) {
        return EVAL_FAIL;
      } else if (eval_ret == EVAL_BREAK) {
        break;
      } else if (eval_ret == EVAL_CONTINUE || eval_ret == EVAL_OK) {
        struct Value expr_value;
        int ret = eval_expression(node->condition, table, &expr_value);
        if (ret == FAIL) {
          return EVAL_FAIL;
        }

        if (expr_value.type != TYPE_INT && expr_value.type != TYPE_FLOAT) {
          printf("runtime error: condition type must be numeric\n");
          return EVAL_FAIL;
        }

        if ((expr_value.type == TYPE_INT && expr_value.int_value == 0)
          || (expr_value.type == TYPE_FLOAT && expr_value.float_value == 0.0)) {
          break;
        }
      }
    }

    return EVAL_OK;
  } else if (node->kind == AST_FOR) {
    if (node->body == NULL) {
      return EVAL_FAIL;
    }

    struct SymbolTable local_runtime_table;
    initialize_symbol_table(&local_runtime_table, table);

    // evaluate init in the for-local scope
    enum EvalResult eval_ret = EVAL_OK;
    if (node->init != NULL) {
      if (node->init->kind == AST_DECLARATION) {
        eval_ret = eval_statement(node->init, &local_runtime_table);
        if (eval_ret == EVAL_FAIL) {
          freeSymbolTableContents(&local_runtime_table);
          return EVAL_FAIL;
        }
      } else {
        struct Value expr_value;
        int ret = eval_expression(node->init, &local_runtime_table, &expr_value);
        if (ret == FAIL) {
          freeSymbolTableContents(&local_runtime_table);
          return EVAL_FAIL;
        }
      }
    }

    int ret;
    while(1) {
      struct Value expr_value = {0};

      if (node->condition != NULL) {
        ret = eval_expression(node->condition, &local_runtime_table, &expr_value);
        if (ret == FAIL) {
          freeSymbolTableContents(&local_runtime_table);
          return EVAL_FAIL;
        }

        if (expr_value.type != TYPE_INT && expr_value.type != TYPE_FLOAT) {
          printf("runtime error: condition type must be numeric\n");
          freeSymbolTableContents(&local_runtime_table);
          return EVAL_FAIL;
        }

        if ((expr_value.type == TYPE_INT && expr_value.int_value == 0)
          || (expr_value.type == TYPE_FLOAT && expr_value.float_value == 0.0)) {
          break;
        }
      }

      eval_ret = eval_statement(node->body, &local_runtime_table);
      if (eval_ret == EVAL_FAIL) {
        freeSymbolTableContents(&local_runtime_table);
        return EVAL_FAIL;
      } else if (eval_ret == EVAL_BREAK) {
        break;
      }

      // EVAL_OK or EVAL_CONTINUE
      if (node->update != NULL) {
        ret = eval_expression(node->update, &local_runtime_table, &expr_value);
        if (ret == FAIL) {
          freeSymbolTableContents(&local_runtime_table);
          return EVAL_FAIL;
        }
      }
    }

    freeSymbolTableContents(&local_runtime_table);
    return EVAL_OK;
  } else if (node->kind == AST_BREAK) {
    return EVAL_BREAK;
  } else if (node->kind == AST_CONTINUE) {
    return EVAL_CONTINUE;
  } else if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      printf("runtime error: print statement missing expression\n");
      return EVAL_FAIL;
    }

    struct Value out_value;
    int ret = eval_expression(node->left, table, &out_value);
    if (ret == FAIL) {
      return EVAL_FAIL;
    }

    if (out_value.type == TYPE_INT) {
      printf("%d\n", out_value.int_value);
    } else if (out_value.type == TYPE_FLOAT) {
      printf("%f\n", out_value.float_value);
    }
    return EVAL_OK;
  } else if (node->kind == AST_EXPRESSION_STATEMENT) {
    if (node->expression == NULL) {
      printf("runtime error: missing expression\n");
      return EVAL_FAIL;
    }

    struct Value out_value;
    int ret = eval_expression(node->expression, table, &out_value);
    if (ret == FAIL) {
      return EVAL_FAIL;
    }

    return EVAL_OK;
  } else if (node->kind == AST_FUNCTION) {
    print_ast_tree(node, 0);
    return EVAL_OK;
  } else {
    printf("runtime error: invalid expression node found: %s\n", typekind_type_name(node->kind));
  }

  return EVAL_FAIL;
}

int eval_program(struct ASTNode * node, struct SymbolTable *table) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind != AST_PROGRAM) {
    return FAIL;
  }

  for (int c=0; c < node->statement_count; c++) {
    enum EvalResult eval_ret = eval_statement(node->statements[c], table);
    if(eval_ret != EVAL_OK) {
      return FAIL;
    }
  }
  return SUCCESS;
}

/******************************************************
 * semantic analysis pass over AST - NO COMPUTE
 ******************************************************/

// variable declaration of type int or float with optional initialization
int analyze_declaration(struct ASTNode *node, struct SymbolTable *sym_table, struct FunctionSymbolTable *func_sym_table, struct SemanticContext *ctx, int global_scope) {
  enum TypeKind declared_type = node->declared_type;

  if (is_supported_type(declared_type) == 0) {
    printf("semantic error: unsupported data type declaration: %s type.\n",
      typekind_type_name(declared_type));
    node->type_kind = TYPE_ERROR;
    return FAIL;
  }

  if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
    printf("semantic error: declaration statement must target a valid variable identifier\n");
    node->type_kind = TYPE_ERROR;
    return FAIL;
  }

  char *identifier = node->left->identifier_name;

  struct FunctionSymbol *func_symbol = NULL;
  struct Symbol *symbol = lookup_symbol_current(sym_table, identifier);
  if (symbol == NULL && global_scope == TRUE && func_sym_table != NULL) {
    func_symbol = lookup_function_symbol(func_sym_table, identifier);
  }

  if (symbol || func_symbol) {
    printf("semantic error: redeclaration of identifier '%s' is illegal in this scope.\n", identifier);
    node->type_kind = TYPE_ERROR;
    return FAIL;
  }

  if (node->right != NULL) {
    enum TypeKind expression_type = analyze_expression(node->right, sym_table);
    if (expression_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (is_assignment_compatible(declared_type, expression_type) == FALSE) {
      printf("semantic error: initializer assignment type mismatch for identifier '%s'\n", identifier);
      printf("semantic error: expected %s but found expression type %s\n",
        typekind_type_name(declared_type), typekind_type_name(expression_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }
  }

  node->left->type_kind = declared_type;

  struct Value value = {0};
  value.type = declared_type;

  // miniC rule: declarations without initializer get default 0 / 0.0
  if (declared_type == TYPE_INT) {
    value.int_value = 0;
  } else if (declared_type == TYPE_FLOAT) {
    value.float_value = 0.0;
  }

  store_symbol(sym_table, declared_type, identifier, &value);
  node->type_kind = declared_type;
  return SUCCESS;
}

enum TypeKind analyze_expression(struct ASTNode *node, struct SymbolTable *table) {

  if (node == NULL) {
    return TYPE_ERROR;
  }

  if (node->kind == AST_NUMBER) {
    if (node->type_kind != TYPE_INT && node->type_kind != TYPE_FLOAT) {
      printf("semantic error: number expression has invalid type\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    return node->type_kind;
  } else if (node->kind == AST_ASSIGNMENT) {

    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("semantic error: left side of an assignment expression must be an identifier\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      printf("semantic error: assignment to undeclared identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    if (is_supported_type(symbol->type) == 0) {
      printf("semantic error: unsupported type: %s for identifier: %s.\n", typekind_type_name(symbol->type), identifier);
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    node->left->type_kind = symbol->type;

    if (node->right == NULL) {
      printf("semantic error: missing assignment value for identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    enum TypeKind expression_type = analyze_expression(node->right, table);
    if (expression_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    if (is_assignment_compatible(symbol->type, expression_type) == FALSE) {
      printf("semantic error: assignment type mismatch for identifier %s\n", identifier);
      printf("semantic error: expected %s but found %s type.\n",
        typekind_type_name(symbol->type), typekind_type_name(expression_type));
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    node->type_kind = symbol->type;
    return node->type_kind;
  } else if (node->kind == AST_UNARY) {
    if (node->left == NULL) {
      printf("semantic error: unary expression missing operand\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    if (node->operator_kind != TOKEN_MINUS && node->operator_kind != TOKEN_PLUS && node->operator_kind != TOKEN_BANG) {
      printf("semantic error: unsupported unary operator %s\n", token_type_name(node->operator_kind));
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    enum TypeKind child_type = analyze_expression(node->left, table);
    if (child_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    if (child_type == TYPE_INT || child_type == TYPE_FLOAT) {
      if (node->operator_kind == TOKEN_BANG) {
        node->type_kind = TYPE_INT;
      } else {
        node->type_kind = child_type;
      }
    } else {
      printf("semantic error: currently minic only supports INT/FLOAT types for operands\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }
    return node->type_kind;
  } else if (node->kind == AST_IDENTIFIER) {
    char *identifier = node->identifier_name;
    if (identifier == NULL) {
      printf("semantic error: identifier is empty?\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      printf("semantic error: use of undeclared identifier %s\n", identifier);
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }
    if (symbol->type == TYPE_ERROR || symbol->type == TYPE_UNKNOWN || symbol->type == TYPE_UNUSED) {
      printf("semantic error: identifier '%s' has no type information\n", identifier);
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }
    node->type_kind = symbol->type;
    return symbol->type;
  } else if (node->kind == AST_BINARY) {

    if (is_supported_binary_operator(node->operator_kind) == 0) {
      printf("semantic error: unsupported binary operator found: %s\n", token_type_name(node->operator_kind));
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    if (node->left == NULL) {
      printf("semantic error: binary operator missing left operand\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    if (node->right == NULL) {
      printf("semantic error: binary operator missing right operand\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    enum TypeKind leftType = analyze_expression(node->left, table);
    enum TypeKind rightType = analyze_expression(node->right, table);
    if (leftType == TYPE_ERROR || rightType == TYPE_ERROR) {
      /*
      printf("semantic error: unsupported types found. Left has: %s and right: %s types \n",
        typekind_type_name(leftType), typekind_type_name(rightType));
      */
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
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
        printf("semantic error: binary arithmetic expression operands have type error.\n");
        node->type_kind = TYPE_ERROR;
        return TYPE_ERROR;
      }
      node->type_kind = TYPE_INT;
      return TYPE_INT;
    } else if (operator_kind == TOKEN_ANDAND || operator_kind == TOKEN_OROR ||
      operator_kind == TOKEN_EQUAL_EQUAL || operator_kind == TOKEN_BANG_EQUAL) {
      if ((leftType != TYPE_INT && leftType  != TYPE_FLOAT) ||
          (rightType != TYPE_INT && rightType  != TYPE_FLOAT)) {
        printf("semantic error: logical/equality operands must be numeric.\n");
        node->type_kind = TYPE_ERROR;
        return TYPE_ERROR;
      }

      node->type_kind = TYPE_INT;
      return TYPE_INT;
    } else if (operator_kind == TOKEN_LESS || operator_kind == TOKEN_GREATER ||
      operator_kind == TOKEN_LESS_EQUAL || operator_kind == TOKEN_GREATER_EQUAL) {
      if ((leftType != TYPE_INT && leftType  != TYPE_FLOAT) ||
          (rightType != TYPE_INT && rightType  != TYPE_FLOAT)) {
        printf("semantic error: comparison operands must be numeric.\n");
        node->type_kind = TYPE_ERROR;
        return TYPE_ERROR;
      }

      node->type_kind = TYPE_INT;
      return TYPE_INT;
    } else if (operator_kind == TOKEN_PERCENT) {
      if (leftType == TYPE_INT && rightType  == TYPE_INT) {
        node->type_kind = TYPE_INT;
        return TYPE_INT;
      } else {
        printf("semantic error: invalid operand to binary expression modulo (%%) requires integer types.\n");
        node->type_kind = TYPE_ERROR;
        return TYPE_ERROR;
      }
    } else {
      printf("semantic error: invalid binary operator found.\n");
      node->type_kind = TYPE_ERROR;
      return TYPE_ERROR;
    }

    return node->type_kind;
  }

  printf("semantic error: unsupported expression ASTNode: %s\n", ast_node_type_name(node->kind));
  node->type_kind = TYPE_ERROR;
  return TYPE_ERROR;
}

int analyze_statement(struct ASTNode *node, struct SymbolTable *table, struct FunctionSymbolTable *func_sym_table, struct SemanticContext *ctx) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_EMPTY) {
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_DECLARATION) {
    return analyze_declaration(node, table, NULL, ctx, FALSE);
  } else if (node->kind == AST_EXPRESSION_STATEMENT) {

    enum TypeKind expression_type = TYPE_UNUSED;
    if (node->expression) {
      expression_type = analyze_expression(node->expression, table);
      if (expression_type == TYPE_ERROR) {
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_BLOCK) {
    struct SymbolTable local_semantic_table;
    initialize_symbol_table(&local_semantic_table, table);

    for (int c=0; c < node->statement_count; c++) {
      if (node->statements[c] == NULL) {
        freeSymbolTableContents(&local_semantic_table);
        return FAIL;
      }

      if (analyze_statement(node->statements[c], &local_semantic_table, NULL, ctx) == FAIL) {
        freeSymbolTableContents(&local_semantic_table);
        return FAIL;
      }
    }

    //printf("Local symbol table after analyzing statement.\n");
    //print_symbol_table(&local_semantic_table);
    freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_IF) {
    if (node->condition == NULL) {
      printf("semantic error: if statement requires an expression to evaluate\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind condition_type = analyze_expression(node->condition, table);
    if (condition_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (condition_type != TYPE_INT && condition_type != TYPE_FLOAT) {
      printf("semantic error: condition expression only supports numeric types but returned: %s\n",
        typekind_type_name(condition_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->then_branch == NULL) {
      printf("semantic error: if statement requires a then statement.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_statement(node->then_branch, table, NULL, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->else_branch != NULL) {
      ret = analyze_statement(node->else_branch, table, NULL, ctx);
      if (ret == FAIL) {
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_WHILE) {
    // push this loop information to the ctx object
    struct ContextFrame ctx_frame;
    ctx_frame.kind = AST_WHILE;
    ctx_frame.prev = ctx->current_ctx;
    ctx->current_ctx = &ctx_frame;

    if (node->condition == NULL) {
      printf("semantic error: while requires an expression to evaluate\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    enum TypeKind condition_type = analyze_expression(node->condition, table);
    if (condition_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    if (condition_type != TYPE_INT && condition_type != TYPE_FLOAT) {
      printf("semantic error: condition type must be numeric, but returned: %s\n",
        typekind_type_name(condition_type));
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    if (node->body == NULL) {
      printf("semantic error: while loop missing body.\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    int ret = analyze_statement(node->body, table, NULL, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    // pop the loop information from the ctx before returning
    ctx->current_ctx = ctx_frame.prev;
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_DO_WHILE) {

    struct ContextFrame ctx_frame;
    ctx_frame.kind = AST_DO_WHILE;
    ctx_frame.prev = ctx->current_ctx;
    ctx->current_ctx = &ctx_frame;

    int ret = analyze_statement(node->body, table, NULL, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    if (node->condition == NULL) {
      printf("semantic error: do-while requires a condition expression\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    enum TypeKind condition_type = analyze_expression(node->condition, table);
    if (condition_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    if (condition_type != TYPE_INT && condition_type != TYPE_FLOAT) {
      printf("semantic error: condition type must be numeric, but returned: %s\n",
        typekind_type_name(condition_type));
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      return FAIL;
    }

    ctx->current_ctx = ctx_frame.prev;
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_FOR) {
    struct ContextFrame ctx_frame;
    ctx_frame.kind = AST_FOR;
    ctx_frame.prev = ctx->current_ctx;
    ctx->current_ctx = &ctx_frame;

    struct SymbolTable local_semantic_table;
    initialize_symbol_table(&local_semantic_table, table);

    if (node->init != NULL) {
      if (node->init->kind == AST_DECLARATION) {
        int ret = analyze_statement(node->init, &local_semantic_table, NULL, ctx);
        if (ret == FAIL) {
          node->type_kind = TYPE_ERROR;
          ctx->current_ctx = ctx_frame.prev;
          freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
          return FAIL;
        }
      } else {
        enum TypeKind init_expr_type = analyze_expression(node->init, &local_semantic_table);
        if (init_expr_type == TYPE_ERROR) {
          node->type_kind = TYPE_ERROR;
          ctx->current_ctx = ctx_frame.prev;
          freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
          return FAIL;
        }
      }
    }

    if (node->condition != NULL) {
      enum TypeKind condition_type = analyze_expression(node->condition, &local_semantic_table);
      if (condition_type == TYPE_ERROR) {
        node->type_kind = TYPE_ERROR;
        ctx->current_ctx = ctx_frame.prev;
        freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
        return FAIL;
      }

      if (condition_type != TYPE_INT && condition_type != TYPE_FLOAT) {
        printf("semantic error: condition type must be numeric, but returned: %s\n",
          typekind_type_name(condition_type));
        node->type_kind = TYPE_ERROR;
        ctx->current_ctx = ctx_frame.prev;
        freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
        return FAIL;
      }
    }

    if (node->update != NULL) {
      enum TypeKind update_expr_type = analyze_expression(node->update, &local_semantic_table);
      if (update_expr_type == TYPE_ERROR) {
        node->type_kind = TYPE_ERROR;
        ctx->current_ctx = ctx_frame.prev;
        freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
        return FAIL;
      }
    }

    if (node->body == NULL) {
      printf("semantic error: for loop missing body.\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
      return FAIL;
    }

    int ret = analyze_statement(node->body, &local_semantic_table, NULL, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_ctx = ctx_frame.prev;
      freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
      return FAIL;
    }

    // pop the parent ctx information from the ctx before returning
    ctx->current_ctx = ctx_frame.prev;

    freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_BREAK) {

    if (!is_inside_loop(ctx)) {
      printf("semantic error: break statement not inside a loop.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_CONTINUE) {

    if (!is_inside_loop(ctx)) {
      printf("semantic error: continue statement not inside a loop.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      printf("semantic error: print statement requires an expression\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind expression_type = analyze_expression(node->left, table);
    if (expression_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_RETURN) {

    // find enclosing function context
    struct ContextFrame * function_ctx = find_enclosing_context(ctx, AST_FUNCTION);
    if (function_ctx == NULL) {
      printf("semantic error: return used outside function.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->expression == NULL) {
      printf("semantic error: non-void function must return a value.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind expression_type = analyze_expression(node->expression, table);
    if (expression_type == TYPE_ERROR) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    // check the type of return expression and function
    enum TypeKind function_return_type =  function_ctx->function_return_type;
    if (is_assignment_compatible(function_return_type, expression_type) == FALSE) {
        printf("semantic error: return expression type mismatch with type of function\n");
        printf("semantic error: expected %s but found expression type %s\n",
          typekind_type_name(function_return_type), typekind_type_name(expression_type));
        node->type_kind = TYPE_ERROR;
        return FAIL;
    }

    node->type_kind = expression_type;
    return SUCCESS;
  }

  printf("semantic error: unsupported statement ASTNode: %s\n", ast_node_type_name(node->kind));
  return FAIL;
}

// only allowed items in the global scope are,
// ;
// variable declaration 
// function definition 
int analyze_global_item(struct ASTNode *node, struct SymbolTable *sym_table, struct FunctionSymbolTable *func_sym_table, struct SemanticContext *ctx) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_EMPTY) {
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_DECLARATION) {
    return analyze_declaration(node, sym_table, func_sym_table, ctx, TRUE);
  } else if (node->kind == AST_FUNCTION) {

    if (!is_supported_type(node->function_type)) {
      printf("semantic error: function '%s', return type is not yet supported.\n", node->function_name);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = store_function_symbol(func_sym_table, node->function_type, node->function_name);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    // Check against the symbol table
    struct Symbol *symbol = lookup_symbol_current(sym_table, node->function_name);
    if (symbol != NULL) {
      printf("semantic error: function '%s', redeclared as different kind of symbol\n", node->function_name);
      printf("semantic error note: previous declaration of '%s', with type '%s'\n", symbol->name, typekind_type_name(symbol->type)); 
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    struct ContextFrame ctx_frame;
    ctx_frame.kind = AST_FUNCTION;
    ctx_frame.function_return_type = node->function_type;
    ctx_frame.prev = ctx->current_ctx;
    ctx->current_ctx = &ctx_frame;

    struct SymbolTable local_semantic_table;
    initialize_symbol_table(&local_semantic_table, sym_table);

    // Check if parameters exists, insert them in to local symbol table
    struct FunctionParam func_param = node->func_param;
    if (func_param.count > 0) {
      for (int i=0; i < func_param.count; i++) {
        struct Param param = func_param.params[i];
        struct Value value = {0};
        struct Symbol *symbol = lookup_symbol_current(&local_semantic_table, param.identifier_name);
        if (symbol != NULL) {
          if (symbol->type == param.type_kind) {
            printf("semantic error: redeclaration of variable ‘%s’\n", param.identifier_name);
          } else {
            printf("semantic error: conflicting types for ‘%s’\n", param.identifier_name);
          }

          freeSymbolTableContents(&local_semantic_table);
          ctx->current_ctx = ctx_frame.prev;
          node->type_kind = TYPE_ERROR;
          return FAIL;
        }
        store_symbol(&local_semantic_table, param.type_kind, param.identifier_name, &value);
      }
    }

    if (node->body != NULL) {
      int ret = analyze_statement(node->body, &local_semantic_table, NULL, ctx);
      if (ret == FAIL) {
        freeSymbolTableContents(&local_semantic_table);
        ctx->current_ctx = ctx_frame.prev;
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    if (statement_guarantees_return(node->body) == FALSE) {
      printf("semantic error: non-void function '%s' may reach end without returning a value.\n", node->function_name);
      freeSymbolTableContents(&local_semantic_table);
      ctx->current_ctx = ctx_frame.prev;
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    // pop the parent ctx information from the ctx before returning
    ctx->current_ctx = ctx_frame.prev;

    node->type_kind = node->function_type;
    freeSymbolTableContents(&local_semantic_table);
    return SUCCESS;
  }

  printf("semantic error: unsupported ASTNode: %s in global scope\n", ast_node_type_name(node->kind));
  node->type_kind = TYPE_UNUSED;
  return FAIL;
}

int analyze_program(struct ASTNode *node, struct SymbolTable *sym_table, struct FunctionSymbolTable *func_table, struct SemanticContext *ctx) {
  if (node == NULL) {
    printf("semantic error: program node is NULL\n");
    return FAIL;
  }

  if (node->kind != AST_PROGRAM) {
    printf("semantic error: analyze_program expected AST_PROGRAM node.\n");
    return FAIL;
  }

  for (int c=0; c < node->statement_count; c++) {
    if (node->statements[c] == NULL) {
      printf("semantic error: no valid program found\n");
      return FAIL;
    }

    if (analyze_global_item(node->statements[c], sym_table, func_table, ctx) == FAIL) {
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
    return TYPE_INT;
  }

  if (is_current_token(p, TOKEN_FLOAT)) {
    expect_token(p, TOKEN_FLOAT);
    return TYPE_FLOAT;
  }

  //printf("error: unsupported data type found\n");
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

    struct ASTNode * node = createNumberNode(numberType, value);
    if (node == NULL) {
      p->has_error = 1;
      return NULL;
    }

    return node;
  }

  return NULL;
}

// IDENTIFIER -> x | _
struct ASTNode * parse_identifier(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  if (is_current_token(p, TOKEN_IDENTIFIER))  {
    struct Token token = p->tokens[p->pos];

    int bytesToCopy = (token.length >= BUFFER_SIZE) ? BUFFER_SIZE-1 : token.length;
    char name[BUFFER_SIZE];
    strncpy(name, token.start, bytesToCopy);
    name[bytesToCopy] = '\0';

    advance_token(p);

    struct ASTNode * node = createIdentifierNode(name);
    if (node == NULL) {
      p->has_error = 1;
      return NULL;
    }

    return node;
  }

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

// unary -> ('-' | '+' | '!') unary | primary
struct ASTNode * parse_unary(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  if (is_current_token(p, TOKEN_MINUS) || is_current_token(p, TOKEN_PLUS) || is_current_token(p, TOKEN_BANG)) {
    struct Token token = current_token(p);
    advance_token(p);
    struct ASTNode * child = parse_unary(p);
    if ( child == NULL ) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing operand expression following '%s' unary operator\n", token_type_name(token.type));
      } else {
        printf("parse error: invalid operand expression following '%s' unary operator\n", token_type_name(token.type));
      }
      return NULL;
    }

    struct ASTNode * node = createUnaryNode(token.type);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(child);
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

// factor -> unary ((STAR | SLASH | PERCENT) unary)*
struct ASTNode * parse_factor(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = NULL;

  left = parse_unary(p);
  if (left == NULL) {
    return NULL;
  }

  while (p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_STAR)) {
      expect_token(p, TOKEN_STAR);  // consume the token
    } else if (is_current_token(p, TOKEN_SLASH)) {
      expect_token(p, TOKEN_SLASH); // consume the token
    } else if (is_current_token(p, TOKEN_PERCENT)) {
      expect_token(p, TOKEN_PERCENT); // consume the token
    } else {
      break;
    }

    struct ASTNode * right = parse_unary(p);
    if (right == NULL) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("parse error: invalid expression following '%s' operator\n", token_type_name(token.type));
      }
      freeASTNode(left);
      return NULL;
    }

    struct ASTNode * node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(left);
      freeASTNode(right);
      return NULL;
    } else {
      left = node;
    }
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

    struct ASTNode * right = parse_factor(p);
    if (right == NULL) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("parse error: invalid expression following '%s' operator\n", token_type_name(token.type));
      }
      freeASTNode(left);
      return NULL;
    }

    struct ASTNode * node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(left);
      freeASTNode(right);
      return NULL;
    } else {
      left = node;
    }
  }

  return left;
}

// comparison -> term ((LESS | GREATER | LESS_OR_EQUAL | GREATER_OR_EQUAL) term)*
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
    } else if (is_current_token(p, TOKEN_LESS_EQUAL)) {
      expect_token(p, TOKEN_LESS_EQUAL);  // consume the token
    } else if (is_current_token(p, TOKEN_GREATER_EQUAL)) {
      expect_token(p, TOKEN_GREATER_EQUAL); // consume the token
    } else {
      break;
    }

    struct ASTNode * right = parse_term(p);
    if (right == NULL) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("parse error: invalid expression following '%s' operator\n", token_type_name(token.type));
      }
      freeASTNode(left);
      return NULL;
    }

    struct ASTNode * node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(left);
      freeASTNode(right);
      return NULL;
    } else {
      left = node;
    }
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

    struct ASTNode * right = parse_comparison(p);
    if (right == NULL) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("parse error: invalid expression following '%s' operator\n", token_type_name(token.type));
      }
      freeASTNode(left);
      return NULL;
    }

    struct ASTNode * node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(left);
      freeASTNode(right);
      return NULL;
    } else {
      left = node;
    }
  }

  return left;
}

// parse_logical_and -> parse_equality (&& parse_equality)*
struct ASTNode * parse_logical_and(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = parse_equality(p);
  if (left == NULL) {
    return NULL;
  }

  while(p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_ANDAND)) {
      expect_token(p, TOKEN_ANDAND);  // consume the token
    } else {
      break;
    }

    struct ASTNode * right = parse_equality(p);
    if (right == NULL) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing expression on right-hand side of '&&' operator\n");
      } else {
        printf("parse error: invalid expression following '&&' operator\n");
      }
      freeASTNode(left);
      return NULL;
    }

    struct ASTNode * node = NULL;
    node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(left);
      freeASTNode(right);
      return NULL;
    } else {
      left = node;
    }
  }

  return left;
}

// parse_logical_or -> parse_logical_and (|| parse_logical_and)*
struct ASTNode * parse_logical_or(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }
  struct ASTNode * left = parse_logical_and(p);
  if (left == NULL) {
    return NULL;
  }

  while(p->has_error == 0) {
    struct Token token = current_token(p);
    if (is_current_token(p, TOKEN_OROR)) {
      expect_token(p, TOKEN_OROR);  // consume the token
    } else {
      break;
    }

    struct ASTNode * right = parse_logical_and(p);
    if (right == NULL) {
      if (p->has_error == 0) {
        p->has_error = 1;
        printf("parse error: missing expression on right-hand side of '||' operator\n");
      } else {
        printf("parse error: invalid expression following '||' operator\n");
      }
      freeASTNode(left);
      return NULL;
    }

    struct ASTNode * node = NULL;
    node = createBinaryNode(token.type, left, right);
    if (node == NULL) {
      p->has_error = 1;
      freeASTNode(left);
      freeASTNode(right);
      return NULL;
    } else {
      left = node;
    }
  }

  return left;
}

// assignment_core -> IDENTIFIER "=" expression
struct ASTNode * parse_assignment_core(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * left = parse_identifier(p);
  if ( left == NULL ) {
    p->has_error = 1;
    return NULL;
  }

  expect_token(p, TOKEN_EQUAL);
  if (p->has_error) {
    printf("parse error: expected '=' after variable identifier in assignment\n");
    freeASTNode(left);
    return NULL;
  }

  struct ASTNode * right = parse_expression(p);
  if (right == NULL ) {
    if (p->has_error == 0) {
      p->has_error = 1;
      printf("parse error: missing expression on right-hand side of assignment\n");
    } else {
      printf("parse error: invalid expression inside assignment\n");
    }
    freeASTNode(left);
    return NULL;
  }

  struct ASTNode * node = createAssignmentNode(left, right);
  if (node == NULL) {
    freeASTNode(left);
    freeASTNode(right);
    p->has_error = 1;
  }

  return node;
}

// assignment -> IDENTIFIER "=" assignment
//            |  logical_or
struct ASTNode * parse_assignment(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_IDENTIFIER) && (is_next_token(p, TOKEN_EQUAL))) {
    node = parse_assignment_core(p);
  } else {
    node = parse_logical_or(p);
  }

  return node;
}

// expression -> assignment
struct ASTNode * parse_expression(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = parse_assignment(p);

  return node;
}

// continue -> continue ";"
struct ASTNode * parse_continue_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume continue token
  expect_token(p, TOKEN_CONTINUE);
  if ( p->has_error ) {
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * node = createContinueNode();
  if (node == NULL) {
    p->has_error = 1;
  }

  return node;
}

// break -> break ";"
struct ASTNode * parse_break_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume break token
  expect_token(p, TOKEN_BREAK);
  if ( p->has_error ) {
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * node = createBreakNode();
  if (node == NULL) {
    p->has_error = 1;
  }

  return node;
}

// print_statement -> PRINT LPAREN expression RPAREN SEMICOLON
struct ASTNode * parse_print_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  expect_token(p, TOKEN_PRINT);
  if ( p->has_error ) {
    return NULL;
  }

  parse_lparen(p);
  if ( p->has_error ) {
    printf("parse error: expected '(' after print statement\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    if (p->has_error == 0) {
      printf("parse error: expected input value to print statement\n");
      p->has_error = 1;
    }
    return NULL;
  }

  parse_rparen(p);
  if (p->has_error) {
    printf("parse error: expected ')' for print statement\n");
    freeASTNode(expr);
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if ( p->has_error ) {
    freeASTNode(expr);
    return NULL;
  }

  struct ASTNode * node = createPrintNode(expr);
  if (node == NULL) {
    freeASTNode(expr);
    p->has_error = 1;
  }

  return node;
}

// block -> '{' statement* '}'
struct ASTNode * parse_block(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * block = createBlockNode();
  if (block == NULL) {
    return NULL;
  }

  expect_token(p, TOKEN_LBRACE); // Start of a block
  if ( p->has_error ) {
    freeASTNode(block);
    return NULL;
  }

  while (p->has_error == 0) {

    if (is_current_token(p, TOKEN_EOF)) {
      printf("parse error: expected } to end block scope but found EOF\n");
      p->has_error=1;
      freeASTNode(block);
      return NULL;
    }

    if (!is_current_token(p, TOKEN_RBRACE)) {
      struct ASTNode *statement = parse_statement(p);
      if (statement == NULL) {
        if (p->has_error == 0) {
          p->has_error=1;
          printf("parse error: invalid or unrecognized statement within block scope.\n");
        } else {
          printf("parse error: invalid statement found within block scope.\n");
        }
        freeASTNode(block);
        return NULL;
      }

      int ret = add_statement_to_block(block, statement);
      if (ret == FAIL) {
        p->has_error=1;
        freeASTNode(statement);
        freeASTNode(block);
        return NULL;
      }

      // continue to read next token
      continue;
    }

    // consume RBRACE token
    expect_token(p, TOKEN_RBRACE);
    if ( p->has_error ) {
      freeASTNode(block);
      return NULL;
    }
    break;
  }

  // return whole AST_BLOCK with a list of statements
  return block;
}

// for_init -> empty | expression | declaration
struct ASTNode * parse_for_init(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  if (is_current_token(p, TOKEN_SEMICOLON)) {
    return NULL;
  }

  struct ASTNode * init_node = NULL;

  // check if its a declaration
  if (is_current_token(p, TOKEN_INT) || is_current_token(p, TOKEN_FLOAT)) {
    enum TypeKind dataType = parse_datatype(p);
    if (dataType == TYPE_ERROR || p->has_error) {
      return NULL;
    }

    // get identifier
    struct ASTNode * identifier = parse_identifier(p);
    if ( identifier == NULL ) {
      return NULL;
    }

    // get expression int x=0;
    if (is_current_token(p, TOKEN_EQUAL)) {
      struct ASTNode * rightValue = NULL;
      expect_token(p, TOKEN_EQUAL);
      rightValue = parse_expression(p);
      if (rightValue == NULL) {
        p->has_error = 1;
        return NULL;
      }

      // create declaratio node
      init_node = createDeclarationNode(dataType, identifier, rightValue);
      if (init_node == NULL) {
        p->has_error = 1;
        freeASTNode(identifier);
        freeASTNode(rightValue);
      }
      return init_node;
    }

    // int x; just create a declaration node with initializer = NULL
    init_node = createDeclarationNode(dataType, identifier, NULL);
    if (init_node == NULL) {
      p->has_error = 1;
      freeASTNode(identifier);
    }
    return init_node;
  } 

  init_node = parse_expression(p);
  if (init_node == NULL) {
    p->has_error = 1;
    return NULL;
  }

  return init_node;
}

// for_statement -> for (init ; expression ; update) statement
struct ASTNode * parse_for_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume FOR token
  expect_token(p, TOKEN_FOR);
  if (p->has_error) {
    return NULL;
  }

  // check and consume opening parenthesis
  parse_lparen(p);
  if (p->has_error) {
    printf("parse error: expected '(' after for keyword\n");
    return NULL;
  }

  // get the init part
  struct ASTNode * init_node = parse_for_init(p);
  if (p->has_error) {
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON); // This consumes ';', error otherwise
  if (p->has_error) {
    printf("parse error: missing ; after initialization in for_statement\n");
    freeASTNode(init_node);
    return NULL;
  }

  // get the condition expression
  struct ASTNode * condition = NULL;
  if (!is_current_token(p, TOKEN_SEMICOLON)) {
    condition = parse_expression(p);
    if (p->has_error) {
      printf("parse error: invalid condition expression inside for_statement.\n");
      freeASTNode(init_node);
      return NULL;
    }
  }

  expect_token(p, TOKEN_SEMICOLON); // This consumes ';', error otherwise
  if (p->has_error) {
    printf("parse error: missing ; after condition in for_statement\n");
    freeASTNode(init_node);
    freeASTNode(condition);
    return NULL;
  }

  // get the update expression
  struct ASTNode * update_node = NULL;
  if (!is_current_token(p, TOKEN_RPAREN)) {
    update_node = parse_expression(p);
    if (p->has_error) {
      printf("parse error: invalid update expression inside for_statement.\n");
      freeASTNode(init_node);
      freeASTNode(condition);
      return NULL;
    }
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if (p->has_error) {
    printf("parse error: missing closing parenthesis in for_statement\n");
    freeASTNode(init_node);
    freeASTNode(condition);
    freeASTNode(update_node);
    return NULL;
  }

  // now parse body statements
  struct ASTNode * body_stmt = parse_statement(p);
  if (body_stmt == NULL) {
    if (p->has_error == 0) {
      p->has_error = 1;
      printf("parse error: missing statement in for body.\n");
    } else {
      printf("parse error: invalid statement in for body.\n");
    }
    freeASTNode(init_node);
    freeASTNode(condition);
    freeASTNode(update_node);
    return NULL;
  }

  // no declaration allowed as the direct for body
  if (body_stmt->kind == AST_DECLARATION) {
    p->has_error = 1;
    printf("parse error: direct declaration is not allowed as body; use a block\n");
    freeASTNode(init_node);
    freeASTNode(condition);
    freeASTNode(update_node);
    freeASTNode(body_stmt);
    return NULL;
  }

  // create ASTNode representing AST_FOR node
  struct ASTNode * node = createForNode(init_node, condition, update_node, body_stmt);
  if (node == NULL) {
    p->has_error = 1;
    freeASTNode(init_node);
    freeASTNode(condition);
    freeASTNode(update_node);
    freeASTNode(body_stmt);
    return NULL;
  }

  return node;
}

// do_statement -> do statement while ( expr );
struct ASTNode * parse_do_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume DO token
  expect_token(p, TOKEN_DO);
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * body_stmt = parse_statement(p);
  if (body_stmt == NULL) {
    if (p->has_error == 0) {
      p->has_error = 1;
      printf("parse error: missing statement in do body.\n");
    } else {
      printf("parse error: invalid statement in do body.\n");
    }
    return NULL;
  }

  // no declaration allowed as the direct do body
  if (body_stmt->kind == AST_DECLARATION) {
    p->has_error = 1;
    printf("parse error: direct declaration is not allowed as body; use a block\n");
    freeASTNode(body_stmt);
    return NULL;
  }

  expect_token(p, TOKEN_WHILE);
  if (p->has_error) {
    freeASTNode(body_stmt);
    return NULL;
  }

  // check and consume opening parenthesis
  parse_lparen(p);
  if (p->has_error) {
    printf("parse error: expected '(' after while statement\n");
    freeASTNode(body_stmt);
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    if (p->has_error == 0) {
      printf("parse error: missing expression in while statement\n");
      p->has_error = 1;
    } else {
      printf("parse error: invalid expression inside while statement condition.\n");
    }
    freeASTNode(body_stmt);
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if (p->has_error) {
    printf("parse error: missing closing parenthesis in while statement\n");
    freeASTNode(expr);
    freeASTNode(body_stmt);
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if (p->has_error) {
    freeASTNode(expr);
    freeASTNode(body_stmt);
    return NULL;
  }

  struct ASTNode * node = createDoWhileNode(expr, body_stmt);
  if (node == NULL) {
    p->has_error = 1;
    freeASTNode(expr);
    freeASTNode(body_stmt);
  }

  return node;
}

struct ASTNode * parse_while_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume WHILE token
  expect_token(p, TOKEN_WHILE);
  if ( p->has_error ) {
    return NULL;
  }

  // check and consume opening parenthesis
  parse_lparen(p);
  if (p->has_error) {
    printf("parse error: expected '(' after while statement\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    if (p->has_error == 0) {
      printf("parse error: missing expression in while statement\n");
      p->has_error = 1;
    } else {
      printf("parse error: invalid expression inside while statement condition.\n");
    }
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if (p->has_error) {
    printf("parse error: missing closing parenthesis in while statement\n");
    freeASTNode(expr);
    return NULL;
  }

  // now parse statements. Empty statement are not allowed
  struct ASTNode * body_stmt = parse_statement(p);
  if (body_stmt == NULL) {
    if (p->has_error == 0) {
      p->has_error = 1;
      printf("parse error: missing statement in while body.\n");
    } else {
      printf("parse error: invalid statement in while body.\n");
    }
    freeASTNode(expr);
    return NULL;
  }

  // no declaration allowed as the direct while body
  if (body_stmt->kind == AST_DECLARATION) {
    p->has_error = 1;
    printf("parse error: direct declaration is not allowed as body; use a block\n");
    freeASTNode(expr);
    freeASTNode(body_stmt);
    return NULL;
  }

  struct ASTNode * node = createWhileNode(expr, body_stmt);
  if (node == NULL) {
    p->has_error = 1;
    freeASTNode(body_stmt);
    freeASTNode(expr);
  }

  return node;
}

// if_statement -> if ( expr ) statement else statement
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

  // check and consume opening parenthesis
  parse_lparen(p);
  if (p->has_error) {
    printf("parse error: expected '(' after if statement\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    p->has_error = 1;
    printf("parse error: missing expression in if statement\n");
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if ( p->has_error ) {
    printf("parse error: missing closing parenthesis in if statement\n");
    freeASTNode(expr);
   return NULL;
  }

  // now parse statements. Empty statement are not allowed
  struct ASTNode * then_stmt = parse_statement(p);
  if (then_stmt == NULL) {
    p->has_error = 1;
    printf("parse error: missing then statement after if condition.\n");
    freeASTNode(expr);
    return NULL;
  }

  // no declaration allowed as the direct then body
  if (then_stmt->kind == AST_DECLARATION) {
    printf("parse error: direct declaration is not allowed as then-body; use a block\n");
    p->has_error = 1;
    freeASTNode(expr);
    freeASTNode(then_stmt);
    return NULL;
  }

  struct ASTNode * else_stmt = NULL;
  // now parse else statement
  if (is_current_token(p, TOKEN_ELSE)) {
    expect_token(p, TOKEN_ELSE); // consume ELSE token
    if (p->has_error) {
      freeASTNode(expr);
      freeASTNode(then_stmt);
      return NULL;
    }

    else_stmt = parse_statement(p);
    if (else_stmt == NULL) {
      printf("parse error: expected statement after else\n");
      p->has_error = 1;
      freeASTNode(expr);
      freeASTNode(then_stmt);
      return NULL;
    }

    if (else_stmt->kind == AST_DECLARATION) {
      printf("parse error: direct declaration is not allowed as else-body; use a block\n");
      p->has_error = 1;
      freeASTNode(expr);
      freeASTNode(then_stmt);
      freeASTNode(else_stmt);
      return NULL;
    }
  }

  struct ASTNode * node = createIfNode(expr, then_stmt, else_stmt);
  if (node == NULL) {
    p->has_error = 1;
    freeASTNode(expr);
    freeASTNode(then_stmt);
    freeASTNode(else_stmt);
  }

  return node;
}

// for_update -> assignment_without_semicolon
// for_init -> assignment_without_semicolon
// for_stmt -> for ( for_init ; expr ; for_update ) statement

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
    return NULL;
  }

  struct ASTNode * rightValue = NULL;

  if (is_current_token(p, TOKEN_EQUAL)) {
    expect_token(p, TOKEN_EQUAL);
    if (p->has_error) {
      freeASTNode(identifier);
      return NULL;
    }
    rightValue = parse_expression(p);
    if (rightValue == NULL) {
      freeASTNode(identifier);
      p->has_error = 1;
      return NULL;
    }
  }

  expect_token(p, TOKEN_SEMICOLON);
  if ( p->has_error ) {
    freeASTNode(identifier);
    freeASTNode(rightValue);
    return NULL;
  }

  struct ASTNode * node = createDeclarationNode(dataType, identifier, rightValue);
  if (node == NULL) {
    freeASTNode(identifier);
    freeASTNode(rightValue);
    p->has_error = 1;
  }

  return node;
}

// return_statement -> return expression ;
struct ASTNode * parse_return_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume RETURN token
  expect_token(p, TOKEN_RETURN);
  if ( p->has_error ) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_SEMICOLON)) {
    expect_token(p, TOKEN_SEMICOLON);
    node = createReturnNode(NULL);
    if (node == NULL) {
      p->has_error = 1;
    }

    return node;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    if (p->has_error == 0) {
      printf("parse error: missing expression in return statement\n");
      p->has_error = 1;
    } else {
      printf("parse error: invalid expression in return statement\n");
    }
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if (p->has_error) {
    freeASTNode(expr);
    return NULL;
  }

  node = createReturnNode(expr);
  if (node == NULL) {
    p->has_error = 1;
    freeASTNode(expr);
  }

  return node;
}

// empty -> ;
struct ASTNode * parse_empty(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_SEMICOLON)) {
    expect_token(p, TOKEN_SEMICOLON);
    node = createEmptyNode();
    if (node == NULL) {
      p->has_error = 1;
    }
  }

  return node;
}

// expression_stmt -> expression ;
struct ASTNode * parse_expression_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if (p->has_error) {
    printf("parser error: missing ';' to end expression\n");
    freeASTNode(expr);
    return NULL;
  }

  struct ASTNode * node = createExpressionStatementNode(expr);
  if (node == NULL) {
    p->has_error = 1;
    freeASTNode(expr);
    return NULL;
  }

  return node;
}

// statement -> empty | declaration | print | block | if | while | break | continue | for_stmt | return | expression_stmt
struct ASTNode * parse_statement(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = NULL;
  if (is_current_token(p, TOKEN_SEMICOLON)) {
    node = parse_empty(p);
  } else if (is_current_token(p, TOKEN_INT) || is_current_token(p, TOKEN_FLOAT)) {
    node = parse_declaration(p);
  } else if (is_current_token(p, TOKEN_PRINT)) {
    node = parse_print_stmt(p);
  } else if (is_current_token(p, TOKEN_LBRACE)) {
    node = parse_block(p);
  } else if (is_current_token(p, TOKEN_IF)) {
    node = parse_if_stmt(p);
  } else if (is_current_token(p, TOKEN_WHILE)) {
    node = parse_while_stmt(p);
  } else if (is_current_token(p, TOKEN_FOR)) {
    node = parse_for_stmt(p);
  } else if (is_current_token(p, TOKEN_DO)) {
    node = parse_do_stmt(p);
  } else if (is_current_token(p, TOKEN_BREAK)) {
    node = parse_break_stmt(p);
  } else if (is_current_token(p, TOKEN_CONTINUE)) {
    node = parse_continue_stmt(p);
  } else if (is_current_token(p, TOKEN_RETURN)) {
    node = parse_return_stmt(p);
  } else {
    node = parse_expression_stmt(p);
  }

  return node;
}

// function_definition -> datatype IDENTIFIER ( ) { statement* }
struct ASTNode * parse_function_definition(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  enum TypeKind dataType = parse_datatype(p);

  if (dataType == TYPE_ERROR || p->has_error) {
    return NULL;
  }

  struct ASTNode * identifier = parse_identifier(p);
  if ( identifier == NULL ) {
    return NULL;
  }

  if (!is_current_token(p, TOKEN_LPAREN)) {
    p->has_error = 1;
    printf("parse error: expected '(' \n");
    freeASTNode(identifier);
    return NULL;
  }

  expect_token(p, TOKEN_LPAREN);

  int param_count = 0;
  struct FunctionParam func_param = {0};
  while ((p->has_error == 0) && (!is_current_token(p, TOKEN_RPAREN))) {
    if (is_current_token(p, TOKEN_INT) || is_current_token(p, TOKEN_FLOAT)) {
      enum TypeKind dataType = parse_datatype(p);
      if (dataType == TYPE_ERROR || p->has_error) { // this shouldn't happen due to out side condition
        freeASTNode(identifier);
        return NULL;
      }

      // get identifier
      if (!is_current_token(p, TOKEN_IDENTIFIER))  {
        struct Token cur_token = current_token(p);
        printf("parse error: missing identifier after the type, found '%s' \n",
          token_type_name(cur_token.type));
        p->has_error = 1;
        freeASTNode(identifier);
        return NULL;
      }

      struct Param param;

      struct Token token = p->tokens[p->pos];
      int bytesToCopy = (token.length >= BUFFER_SIZE) ? BUFFER_SIZE-1 : token.length;
      char name[BUFFER_SIZE];
      strncpy(param.identifier_name, token.start, bytesToCopy);
      param.identifier_name[bytesToCopy] = '\0';

      param.type_kind = dataType;
      if (param_count >= MAX_PARAMS) {
        printf("parse error: we are out of space to add additional parameters of function: %s\n",
          identifier->identifier_name);
        p->has_error = 1;
        freeASTNode(identifier);
        return NULL;
      }

      func_param.params[param_count] = param;
      param_count++;
      func_param.count = param_count;

      advance_token(p);

      if (is_current_token(p, TOKEN_COMMA)) {
        if (is_next_token(p, TOKEN_INT) || is_next_token(p, TOKEN_FLOAT)) {
          expect_token(p, TOKEN_COMMA);
        } else {
          printf("parse error: expected parameter type after ','\n");
          p->has_error = 1;
          freeASTNode(identifier);
          return NULL;
        }
      }
    } else {
      p->has_error = 1;
      freeASTNode(identifier);
      return NULL;
    }
  }

  expect_token(p, TOKEN_RPAREN);

  if (!is_current_token(p, TOKEN_LBRACE)) {
    printf("parse error: expected '{' \n");
    p->has_error = 1;
    freeASTNode(identifier);
    return NULL;
  }

  struct ASTNode * stmt_block = parse_block(p);
  if (stmt_block == NULL) {
    freeASTNode(identifier);
    p->has_error = 1;
    return NULL;
  }

  // create function definition block
  struct ASTNode * node = createFunctionDefinitionNode(dataType, identifier->identifier_name, stmt_block);
  if (node == NULL) {
    freeASTNode(stmt_block);
    p->has_error = 1;
  }

  node->func_param = func_param;
  return node;
}

// program -> declaration
//         | function_definition
struct ASTNode * parse_program(struct Parser *p) {

  struct ASTNode * program = createProgramNode();
  if (program == NULL) {
    p->has_error = 1;
    return NULL;
  }

  while ((p->has_error == 0) && (!is_current_token(p, TOKEN_EOF))) {
    struct ASTNode * node = NULL;
    if (!is_current_token(p, TOKEN_INT) && !is_current_token(p, TOKEN_FLOAT)) {
      printf("error: expected 'int' or 'float' data type but found invalid token in global scope\n");
      p->has_error = 1;
      break;
    }

    if (!is_next_token(p, TOKEN_IDENTIFIER)) {
      printf("error: expected valid identifier but found invalid token in global scope\n");
      p->has_error = 1;
      break;
    }

    if (is_nth_token(p, 2, TOKEN_SEMICOLON) || is_nth_token(p, 2, TOKEN_EQUAL)) {
      node = parse_declaration(p);
      if (node == NULL || p->has_error) {
        printf("parse error: invalid global declaration.\n");
        p->has_error = 1;
        break;
      }
    } else if (is_nth_token(p, 2, TOKEN_LPAREN)) {
      node = parse_function_definition(p);
      if (node == NULL || p->has_error) {
        printf("parse error: invalid function definition.\n");
        p->has_error = 1;
        break;
      }
    } else {
      printf("parse error: expected declaration or function definition\n");
      p->has_error = 1;
      break;
    }

    // add ASTNode representing a declararation or function definition node to the program node
    int ret = add_node_to_program(program, node);
    if (ret == FAIL) {
      p->has_error = 1;
      freeASTNode(program);
      return NULL;
    }
  }

  if (p->has_error) {
    freeASTNode(program);
    return NULL;
  }

  return program;
}

int main(void) {
  /*
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
  */

  struct Token tokens[MAX_TOKENS];
  int token_count = 0;

  struct stat istat;
  int ret = stat("./test.c", &istat);
  if (ret < 0) {
    perror("stat");
    exit(ret);
  }
  char *icode = (char *)malloc(istat.st_size+1);
  if (icode == NULL) {
    printf("error: could not allocate buffer to read input file\n");
    exit(-1);
  }
  int ifd = open("./test.c", O_RDONLY);
  if (ifd < 0) {
    perror("open");
    exit(-1);
  }
  ssize_t rfd = read(ifd, (char *)icode, istat.st_size);
  if (rfd < 0) {
    perror("read");
    exit(-1);
  }
  icode[istat.st_size] = '\0';

  printf("Input: %s\n", icode);

  // Lexer or Tokenizer
  printf("\n\n--> 1. Tokenizer PHASE: Start\n");
  int result = tokenize(icode, tokens, &token_count);
  if (result == FAIL) {
    printf("--> TOKENIZE PHASE: FAILED\n");
    exit(-1);
  }

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
  p.token_count = token_count;
  p.pos = 0;
  p.has_error = 0;

  printf("\n\n--> 2. PARSING PHASE: Start\n");
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
  printf("\n\n--> 3. SEMANTIC CHECK PHASE: Start\n");
  printf("Initialize symbol table for Semantic check\n");
  struct SymbolTable semantic_table;
  struct FunctionSymbolTable function_symbol_table;

  initialize_symbol_table(&semantic_table, NULL);
  initialize_function_symbol_table(&function_symbol_table);

  struct SemanticContext semantic_ctx;
  semantic_ctx.current_ctx = NULL;

  result = analyze_program(program, &semantic_table, &function_symbol_table, &semantic_ctx);

  if (result == FAIL) {
    printf("--> SEMANTIC CHECK PHASE: FAILED\n");
    print_debug_output(program, &semantic_table, &function_symbol_table);
    freeSymbolTableContents(&semantic_table);
    freeASTNode(program);
    free(icode);
    exit(-1);
  }

  printf("--> SEMANTIC CHECK PHASE: PASS\n");
  print_debug_output(program, &semantic_table, &function_symbol_table);
  freeSymbolTableContents(&semantic_table);
  printf("=====================================================\n");

  // Program is semantically okay so go ahead and evaluate it
  printf("\n\n--> 4. EVALUATION CHECK PHASE: Start\n");
  printf("Initialize symbol table for Evaluation check\n");
  struct SymbolTable runtime_table;
  initialize_symbol_table(&runtime_table, NULL);
  result = eval_program(program, &runtime_table);
  printf("Global symbol table after evaluation phase\n");
  print_symbol_table(&runtime_table);
  if (result == 0) {
    printf("--> EVALUATION CHECK PHASE: FAILED\n");
    freeSymbolTableContents(&runtime_table);
    freeASTNode(program);
    free(icode);
    exit(-1);
  }
  printf("--> EVALUATION CHECK PHASE: PASS\n");

  freeSymbolTableContents(&runtime_table);
  freeASTNode(program);
  free(icode);
  return 0;
}
