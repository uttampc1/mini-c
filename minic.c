/******************************************************************************
miniC - 
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SUCCESS 1
#define FAIL    0

#define BUFFER_SIZE 1024
#define MAX_SYMBOLS 1024
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
  AST_WHILE,
  AST_FOR,
  AST_BREAK,
  AST_CONTINUE,
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

enum EvalResult {
  EVAL_FAIL,
  EVAL_OK,
  EVAL_BREAK,
  EVAL_CONTINUE
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
  struct ASTNode * init;
  struct ASTNode * update;
  struct ASTNode * condition;
  struct ASTNode * body;
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

/* Following two structures are used during semantic analysis phase */
struct LoopFrame {
  enum ASTNodeKind kind;
  struct LoopFrame *prev;
};

struct SemanticContext {
  struct LoopFrame *current_loop;
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
struct ASTNode * createWhileNode(struct ASTNode * expr, struct ASTNode * body_stmt);
struct ASTNode * createBlockNode();
struct ASTNode * createProgramNode();

enum TypeKind analyze_expression_type(struct ASTNode *node, struct SymbolTable *table);
int analyze_expression(struct ASTNode *node, struct SymbolTable *table);
int analyze_statement(struct ASTNode *node, struct SymbolTable *table, struct SemanticContext *ctx); 
int analyze_program(struct ASTNode *node, struct SymbolTable *table, struct SemanticContext *ctx);

enum EvalResult eval_statement(struct ASTNode * root, struct SymbolTable *table);

struct Symbol  * lookup_symbol_current(struct SymbolTable *table, char *name); // current scope search
struct Symbol  * lookup_symbol_visible(struct SymbolTable *table, char *name); // globalc scope search
struct ASTNode * parse_program(struct Parser *p);
struct ASTNode * parse_block(struct Parser *p);
struct ASTNode * parse_statement(struct Parser *p);
struct ASTNode * parse_expression(struct Parser *p);
struct ASTNode * parse_primary(struct Parser *p);
struct ASTNode * parse_term(struct Parser *p);

int add_statement_to_program(struct ASTNode * program, struct ASTNode * statement);
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
    case AST_FOR:
      return "FOR";
    case AST_BREAK:
      return "BREAK";
    case AST_CONTINUE:
      return "CONTINUE";
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
    case TOKEN_BREAK:
      return "BREAK";
    case TOKEN_CONTINUE:
      return "CONTINUE";
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
int add_token(struct Token tokens[], int *token_count, enum TokenKind type, char *start, int length) {
  struct Token t;
  t.type = type;
  t.start = start;
  t.length = length;
  if (*token_count < BUFFER_SIZE) {
    tokens[*token_count] = t;
    *token_count = *token_count + 1;
    return SUCCESS;
  } else {
    printf("error: no more tokens will be accepted. OOM\n");
    return FAIL;
  }
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
    printf("%s %s\n", ast_node_type_name(t->kind), typekind_type_name(t->type_kind));
    for (int c=0; c < t->statement_count; c++) {
      print_ast_tree(t->statements[c], depth+1);
    }
    return;
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
        printf("error: unexpected character '&' after '&'\n");
        return FAIL;
      }
    } else if (is_or(code[idx])) {
      if (code[idx+1] != '\0' && is_or(code[idx + 1])) {
        ret = add_token(tokens, token_count, TOKEN_OROR, code + idx, 2);
        if (ret == SUCCESS) {
          idx += 2;
        }
      } else {
        printf("error: unexpected character '|' after '|'\n");
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
      } else if ((len == 5) && strncmp(code+start, "break", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_BREAK, code + start, len);
      } else if ((len == 8) && strncmp(code+start, "continue", len) == 0) {
        ret = add_token(tokens, token_count, TOKEN_CONTINUE, code + start, len);
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
int is_next_token(struct Parser *p, enum TokenKind type) {
  struct Token t = p->tokens[p->pos+1];
  if (t.type == type) {
    return 1;
  }
  return 0;
}

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
  node->init = NULL;
  node->update = NULL;
  node->condition = NULL;
  node->body = NULL;
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

struct ASTNode * createBreakNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for break ASTNode\n");
    return NULL;
  }

  node->kind = AST_BREAK;
  return node;
}

struct ASTNode * createContinueNode() {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for continue ASTNode\n");
    return NULL;
  }

  node->kind = AST_CONTINUE;
  return node;
}

struct ASTNode * createWhileNode(struct ASTNode * condition, struct ASTNode * body) {
  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for while ASTNode\n");
    return NULL;
  }

  node->kind = AST_WHILE;
  node->condition = condition;
  node->body = body;
  return node;
}

struct ASTNode * createForNode(struct ASTNode * init, struct ASTNode *  condition, 
  struct ASTNode * update, struct ASTNode * body) {

  struct ASTNode * node = createASTNode();
  if (node == NULL) {
    printf("error: couldn't allocate memory for 'for' ASTNode\n");
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
    printf("error: couldn't allocate memory for if ASTNode\n");
    return NULL;
  }

  node->kind = AST_IF;
  node->condition = condition;
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

// helper: add new statement to the program node.
int add_statement_to_program(struct ASTNode * node, struct ASTNode * statement) {
  struct ASTNode * program = node;

  // we can add few checks
  // program != NULL
  // program->kind == AST_PROGRAM
  // statement != NULL
  if ( program && (program->kind == AST_PROGRAM) && (statement != NULL) ) {
    int count = program->statement_count;

    if (count < BUFFER_SIZE) {
      program->statements[count] = statement;
      program->statement_count = count + 1;
    } else {
      printf("error: we are running out of space to add additional statements in a program\n");
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
          printf("error: division by zero.\n");
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
          printf("error: modulo by zero.\n");
          return FAIL;
        }
        out_value->int_value = leftNum.int_value % rightNum.int_value;
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

enum EvalResult eval_statement(struct ASTNode * root, struct SymbolTable *table) {
  struct ASTNode * node = root;
  if (node == NULL) {
    return EVAL_FAIL;
  }

  if (node->kind == AST_DECLARATION) {
    enum TypeKind declared_variable_type = node->declared_variable_type;
    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("runtime error: declaration must have an identifier.\n");
      return EVAL_FAIL;
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
      printf("runtime error: redeclaration of identifier %s\n", identifier);
      return EVAL_FAIL;
    }

    if (node->right) {
      int ret = eval_expression(node->right, table, &out_value);
      if (ret == FAIL) {
       return EVAL_FAIL;
      }
    }

    if (is_assignment_compatible(declared_variable_type, out_value.type) == 0) {
      printf("runtime error: initializer assignment type mismatch for declared identifier %s\n", identifier);
      printf("runtime error: expected %s but found %s type.\n",
        typekind_type_name(declared_variable_type), typekind_type_name(out_value.type));
      return EVAL_FAIL;
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
      return EVAL_FAIL;
    }

    // commit to the runtime symbol map
    store_symbol(table, declared_variable_type, identifier, &out_value);
    return EVAL_OK;
  } else if (node->kind == AST_ASSIGNMENT) {
    char *identifier = node->left->identifier_name;
    struct Symbol *symbol = lookup_symbol_visible(table, identifier);
    if (symbol == NULL) {
      printf("runtime error: assignment targeted to undeclared identifier '%s'\n", identifier);
      return EVAL_FAIL;
    }

    struct Value out_value = {0};
    int ret = eval_expression(node->right, table, &out_value);
    if (ret == FAIL) {
      return EVAL_FAIL;
    }

    if (is_assignment_compatible(symbol->type, out_value.type) == 0) {
      printf("runtime error: assignment type mismatch for declared identifier %s\n", identifier);
      printf("runtime error: expected %s but found %s type.\n",
        typekind_type_name(symbol->type), typekind_type_name(out_value.type));
      return EVAL_FAIL;
    }

    if (symbol->type == TYPE_FLOAT && out_value.type == TYPE_INT) {
      out_value.type = symbol->type;
      out_value.float_value = (double)out_value.int_value;
      out_value.int_value = 0;
    }

    symbol->value = out_value;
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

    //print_symbol_table(&local_runtime_table);
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
        printf("error: condition type must be numeric\n");
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
  } else if (node->kind == AST_FOR) {
    if (node->init == NULL) {
      return EVAL_FAIL;
    }

    if (node->condition == NULL) {
      return EVAL_FAIL;
    }

    if (node->update == NULL) {
      return EVAL_FAIL;
    }

    if (node->body == NULL) {
      return EVAL_FAIL;
    }

    // evaluate initialization - table should have updated value for the symbol
    enum EvalResult eval_ret = eval_statement(node->init, table);
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
        printf("error: condition type must be numeric\n");
        return EVAL_FAIL;
      }

      if ((expr_value.type == TYPE_INT && expr_value.int_value == 0)
        || (expr_value.type == TYPE_FLOAT && expr_value.float_value == 0.0)) {
        break;
      }

      eval_ret = eval_statement(node->body, table);
      if (eval_ret == EVAL_FAIL) {
        return EVAL_FAIL;
      } else if (eval_ret == EVAL_BREAK) {
        break;
      }

      // EVAL_OK or EVAL_CONTINUE
      eval_ret = eval_statement(node->update, table);
      if (eval_ret == EVAL_FAIL) {
        return EVAL_FAIL;
      }
    }

    return EVAL_OK; //uttam
  } else if (node->kind == AST_BREAK) {
    return EVAL_BREAK;
  } else if (node->kind == AST_CONTINUE) {
    return EVAL_CONTINUE;
  } else if (node->kind == AST_PRINT) {
    if (node->left == NULL) {
      printf("error: print statement missing expression\n");
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

// type check over AST - NO COMPUTE
int is_assignment_compatible(enum TypeKind target_type, enum TypeKind source_type) {
  if (target_type == source_type) {
    return 1;
  }

  if ((target_type == TYPE_FLOAT) && (source_type == TYPE_INT)) {
    return 1;
  }

  // Any other combination is incompatible
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
    || operator_kind == TOKEN_PERCENT
    || operator_kind == TOKEN_LESS
    || operator_kind == TOKEN_LESS_EQUAL
    || operator_kind == TOKEN_GREATER
    || operator_kind == TOKEN_GREATER_EQUAL
    || operator_kind == TOKEN_EQUAL_EQUAL
    || operator_kind == TOKEN_BANG_EQUAL
    || operator_kind == TOKEN_ANDAND
    || operator_kind == TOKEN_OROR) {

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
    if (is_supported_binary_operator(node->operator_kind) == 0) {
      printf("error: unsupported binary operator found: %s\n", token_type_name(node->operator_kind));
      return TYPE_ERROR;
    }

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
    } else if (operator_kind == TOKEN_PERCENT) {
      if (leftType == TYPE_INT && rightType  == TYPE_INT) {
        return TYPE_INT;
      } else {
        printf("error: modulo operands must be of int type.\n");
        return TYPE_ERROR;
      }
    } else {
      int left_numeric = (leftType == TYPE_INT || leftType == TYPE_FLOAT);
      int right_numeric = (rightType == TYPE_INT || rightType == TYPE_FLOAT);
      if (!left_numeric || !right_numeric) {
        printf("error: comparison operands must be numeric.\n");
        return TYPE_ERROR;
      }
      return TYPE_INT;
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

  printf("error: unsupported expression for type check analysis: %s\n", ast_node_type_name(node->kind));
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

    if (node->operator_kind != TOKEN_MINUS && node->operator_kind != TOKEN_PLUS && node->operator_kind != TOKEN_BANG) {
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
      if (node->operator_kind == TOKEN_BANG) {
        node->type_kind = TYPE_INT;
      } else {
        node->type_kind = child_type;
      }
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
    } else if (operator_kind == TOKEN_PERCENT) {
      if (leftType == TYPE_INT && rightType  == TYPE_INT) {
        return TYPE_INT;
      } else {
        printf("error: invalid operand to binary expression modulo (%%) requires integer types.\n");
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    } else {
      int left_numeric = (leftType == TYPE_INT || leftType == TYPE_FLOAT);
      int right_numeric = (rightType == TYPE_INT || rightType == TYPE_FLOAT);
      if (!left_numeric || !right_numeric) {
        printf("error: comparison operands must be numeric.\n");
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
      node->type_kind = TYPE_INT;
    }

    return SUCCESS;
  }

  printf("error: unsupported expression ASTNode: %s\n", ast_node_type_name(node->kind));
  return FAIL;
}

int analyze_statement(struct ASTNode *node, struct SymbolTable *table, struct SemanticContext *ctx) {
  if (node == NULL) {
    return FAIL;
  }

  if (node->kind == AST_DECLARATION) {
    enum TypeKind declared_variable_type = node->declared_variable_type;

    if (is_supported_type(declared_variable_type) == 0) {
      printf("error: unsupported data type declaration: %s type.\n",
        typekind_type_name(declared_variable_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->left == NULL || node->left->kind != AST_IDENTIFIER) {
      printf("error: declarationi statement must target a valid variable identifier\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    char *identifier = node->left->identifier_name;

    struct Symbol *symbol = lookup_symbol_current(table, identifier);
    if (symbol) {
      printf("error: redeclaration of identifier '%s' is illegal in this scope.\n", identifier);
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->right != NULL) {
      int ret = analyze_expression(node->right, table);
      if (ret == FAIL) {
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }

      enum TypeKind rhs_expr_type = analyze_expression_type(node->right, table);
      if (rhs_expr_type == TYPE_ERROR) {
        //printf("error: RHS expression has unsupported type: %s\n", typekind_type_name(rhs_expr_type));
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }

      if (is_assignment_compatible(declared_variable_type, rhs_expr_type) == 0) {
        printf("error: intializer assignment type mismatch for identifier '%s'\n", identifier);
        printf("error: expected %s but found experssion type %s\n",
          typekind_type_name(declared_variable_type), typekind_type_name(rhs_expr_type));
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    node->left->type_kind = declared_variable_type;

    struct Value value = {0};
    value.type = declared_variable_type;

    // miniC rule: declarations without initializer get default 0 / 0.0
    if (declared_variable_type == TYPE_INT) {
      value.int_value = 0;
    } else if (declared_variable_type == TYPE_FLOAT) {
      value.float_value = 0.0;
    }

    store_symbol(table, declared_variable_type, identifier, &value);
    node->type_kind = declared_variable_type;
    return SUCCESS;
  } else if (node->kind == AST_ASSIGNMENT) {

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
      //printf("error: RHS expression has unsupported type: %s\n", typekind_type_name(rhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (is_assignment_compatible(symbol->type, rhs_expr_type) == 0) {
      printf("error: assignment type mismatch for identifier %s\n", identifier);
      printf("error: expected %s but found %s type.\n",
        typekind_type_name(symbol->type), typekind_type_name(rhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->type_kind = symbol->type;
    return SUCCESS;
  } else if (node->kind == AST_BLOCK) {
    struct SymbolTable local_semantic_table;
    initialize_symbol_table(&local_semantic_table, table);

    for (int c=0; c < node->statement_count; c++) {
      if (node->statements[c] == NULL) {
        freeSymbolTableContents(&local_semantic_table);
        return FAIL;
      }

      if (analyze_statement(node->statements[c], &local_semantic_table, ctx) == FAIL) {
        freeSymbolTableContents(&local_semantic_table);
        return FAIL;
      }
    }

    printf("Local symbol table after analyzing statement.\n");
    print_symbol_table(&local_semantic_table);
    freeSymbolTableContents(&local_semantic_table); // We do not need this anymore
    return SUCCESS;
  } else if (node->kind == AST_IF) {
    if (node->condition == NULL) {
      printf("error: if statement requires an expression to evaluate\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_expression(node->condition, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind lhs_expr_type = analyze_expression_type(node->condition, table);
    if (lhs_expr_type == TYPE_ERROR) {
      //printf("error: condition expression returned unsupported type: %s\n", typekind_type_name(lhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (lhs_expr_type != TYPE_INT && lhs_expr_type != TYPE_FLOAT) {
      printf("error: condition expression only supports numeric types but returned: %s\n", typekind_type_name(lhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->then_branch == NULL) {
      printf("error: if statement requires a then statement.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    ret = analyze_statement(node->then_branch, table, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->else_branch != NULL) {
      ret = analyze_statement(node->else_branch, table, ctx);
      if (ret == FAIL) {
        node->type_kind = TYPE_ERROR;
        return FAIL;
      }
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_WHILE) {
    if (node->condition == NULL) {
      printf("error: while requires an expression to evaluate\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    int ret = analyze_expression(node->condition, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    enum TypeKind lhs_expr_type = analyze_expression_type(node->condition, table);
    if (lhs_expr_type == TYPE_ERROR) {
      printf("error: condition expression returned unsupported type: %s\n", typekind_type_name(lhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (lhs_expr_type != TYPE_INT && lhs_expr_type != TYPE_FLOAT) {
      printf("error: condition type must be numeric, but returned: %s\n", typekind_type_name(lhs_expr_type));
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    if (node->body == NULL) {
      printf("error: while loop missing body.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    // push this loop information to the ctx object
    struct LoopFrame loop_frame;
    loop_frame.kind = AST_WHILE;
    loop_frame.prev = ctx->current_loop;
    ctx->current_loop = &loop_frame;

    ret = analyze_statement(node->body, table, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    // pop the loop information from the ctx before returning
    ctx->current_loop = ctx->current_loop->prev;

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_FOR) {

    // Handle init condition as an assignment
    if (node->init == NULL) {
      printf("error: for requires an initialization statement.\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    struct LoopFrame loop_frame;
    loop_frame.kind = AST_FOR;
    loop_frame.prev = ctx->current_loop;
    ctx->current_loop = &loop_frame;

    int ret = analyze_statement(node->init, table, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    // Handle condition like an expression
    if (node->condition == NULL) {
      printf("error: for requires an expression to evaluate\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    ret = analyze_expression(node->condition, table);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    enum TypeKind condition_type = analyze_expression_type(node->condition, table);
    if (condition_type == TYPE_ERROR) {
      printf("error: condition expression returned unsupported type: %s\n", typekind_type_name(condition_type));
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    if (condition_type != TYPE_INT && condition_type != TYPE_FLOAT) {
      printf("error: condition type must be numeric, but returned: %s\n", typekind_type_name(condition_type));
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    // Handle update like an assignment
    if (node->update == NULL) {
      printf("error: for requires a update expression\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    ret = analyze_statement(node->update, table, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    // Handle body like an block
      printf("error: for loop missing body.\n");
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    ret = analyze_statement(node->body, table, ctx);
    if (ret == FAIL) {
      node->type_kind = TYPE_ERROR;
      ctx->current_loop = ctx->current_loop->prev;
      return FAIL;
    }

    // pop the loop information from the ctx before returning
    ctx->current_loop = ctx->current_loop->prev;

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_BREAK) {
    // if ctx object does not have a loop_frame then this is a sematic error
    // since break can not be out side the loop construct, while, for, do-while
    if (ctx->current_loop == NULL) {
      printf("error: break statement not inside a loop, semantic error\n");
      return FAIL;
    }
    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  } else if (node->kind == AST_CONTINUE) {
    // if ctx object does not have a loop_frame then this is a sematic error
    // since continue can not be out side the loop construct, while, for, do-while
    if (ctx->current_loop == NULL) {
      printf("error: continue statement not inside a loop, semantic error\n");
      return FAIL;
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
      //printf("error: print expression has unsupported type\n");
      node->type_kind = TYPE_ERROR;
      return FAIL;
    }

    node->type_kind = TYPE_UNUSED;
    return SUCCESS;
  }

  printf("error: unsupported statement ASTNode: %s\n", ast_node_type_name(node->kind));
  return FAIL;
}

int analyze_program(struct ASTNode *node, struct SymbolTable *table, struct SemanticContext *ctx) {
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

    if (analyze_statement(node->statements[c], table, ctx) == FAIL) {
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
    printf("Expected an identifier\n");
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
        printf("error: missing operand expression following '%s' unary operator\n", token_type_name(token.type));
      } else {
        printf("error: invalid operand expression following '%s' unary operator\n", token_type_name(token.type));
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
        printf("error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("error: invalid expression following '%s' operator\n", token_type_name(token.type));
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
        printf("error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("error: invalid expression following '%s' operator\n", token_type_name(token.type));
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
        printf("error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("error: invalid expression following '%s' operator\n", token_type_name(token.type));
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
        printf("error: missing expression on right-hand side of '%s' operator\n", token_type_name(token.type));
      } else {
        printf("error: invalid expression following '%s' operator\n", token_type_name(token.type));
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
        printf("error: missing expression on right-hand side of '&&' operator\n");
      } else {
        printf("error: invalid expression following '&&' operator\n");
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
        printf("error: missing expression on right-hand side of '||' operator\n");
      } else {
        printf("error: invalid expression following '||' operator\n");
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

// expression -> parse_logical_or
struct ASTNode * parse_expression(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = parse_logical_or(p);

  return node;
}

// continue -> continue
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

// break -> break
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

// assignment -> IDENTIFIER EQUAL expression
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
    printf("error: expected '=' after variable identifier in assignment statement\n");
    freeASTNode(left);
    return NULL;
  }

  struct ASTNode * right = parse_expression(p);
  if (right == NULL ) {
    if (p->has_error == 0) {
      p->has_error = 1;
      printf("error: missing expression on right-hand side of assignment statement\n");
    } else {
      printf("error: invalid expression inside assignment statement\n");
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

// assignment -> IDENTIFIER EQUAL expression SEMICOLON
struct ASTNode * parse_assignment(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  struct ASTNode * node = parse_assignment_core(p);
  if (node == NULL) {
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON);
  if (p->has_error) {
    printf("error: expected ';' to terminate assignment statement\n");
    freeASTNode(node);
    return NULL;
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

  parse_lparen(p);
  if ( p->has_error ) {
    printf("error: expected '(' after print statement\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    if (p->has_error == 0) {
      printf("error: expected input value to print statement\n");
      p->has_error = 1;
    }
    return NULL;
  }

  parse_rparen(p);
  if (p->has_error) {
    printf("error: expected ')' for print statement\n");
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
      printf("error: expected } to end block scope but found EOF\n");
      p->has_error=1;
      freeASTNode(block);
      return NULL;
    }

    if (!is_current_token(p, TOKEN_RBRACE)) {
      struct ASTNode *statement = parse_statement(p);
      if (statement == NULL) {
        if (p->has_error == 0) {
          p->has_error=1;
          printf("error: invalid or unrecognized statement within block scope.\n");
        } else {
          printf("error: invalid statement found within block scope.\n");
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

// for_statement -> for (init ; expression ; update) statement
struct ASTNode * parse_for_stmt(struct Parser *p) {
  if (p->has_error) {
    return NULL;
  }

  // consume FOR token
  expect_token(p, TOKEN_FOR);
  if ( p->has_error ) {
    return NULL;
  }

  // check and consume opening parenthesis
  parse_lparen(p);
  if (p->has_error) {
    printf("error: expected '(' after for keyword\n");
    return NULL;
  }

  // get the init part
  struct ASTNode * init_node = parse_assignment_core(p);
  if (init_node == NULL) {
    if (p->has_error == 0) {
      printf("error: missing initialization assignment in for_statement\n");
      p->has_error = 1;
    } else {
      printf("error: invalid initialization in for_statement\n");
    }
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON); // This consumes ';', error otherwise
  if (p->has_error) {
    printf("error: missing ; after initialization assignment in for_statement\n");
    freeASTNode(init_node);
    return NULL;
  }

  // get the condition expression
  struct ASTNode * condition = parse_expression(p);
  if (condition == NULL) {
    if (p->has_error == 0) {
      printf("error: missing condition in for_statement\n");
      p->has_error = 1;
    } else {
      printf("error: invalid condition inside for_statement condition.\n");
    }
    freeASTNode(init_node);
    return NULL;
  }

  expect_token(p, TOKEN_SEMICOLON); // This consumes ';', error otherwise
  if (p->has_error) {
    printf("error: missing ; after condition in for_statement\n");
    freeASTNode(init_node);
    freeASTNode(condition);
    return NULL;
  }

  // get the update expression
  struct ASTNode * update_node = parse_assignment_core(p);
  if (update_node == NULL) {
    if (p->has_error == 0) {
      printf("error: missing update expression in for_statement\n");
      p->has_error = 1;
    } else {
      printf("error: invalid update expression inside for_statement.\n");
    }
    freeASTNode(init_node);
    freeASTNode(condition);
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if (p->has_error) {
    printf("error: missing closing parenthesis in for_statement\n");
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
      printf("error: missing statement in for body.\n");
    } else {
      printf("error: invalid statement in for body.\n");
    }
    freeASTNode(init_node);
    freeASTNode(condition);
    freeASTNode(update_node);
    return NULL;
  }

  // no declaration allowed as the directy for body
  if (body_stmt->kind == AST_DECLARATION) {
    p->has_error = 1;
    printf("error: direct declaration is not allowed as body; use a block\n");
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

// while_statement -> while ( expr ) statement
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
    printf("error: expected '(' after while statement\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    if (p->has_error == 0) {
      printf("error: missing expression in while statement\n");
      p->has_error = 1;
    } else {
      printf("error: invalid expression inside while statement condition.\n");
    }
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if (p->has_error) {
    printf("error: missing closing parenthesis in while statement\n");
    freeASTNode(expr);
    return NULL;
  }

  // now parse statements. Empty statement are not allowed
  struct ASTNode * body_stmt = parse_statement(p);
  if (body_stmt == NULL) {
    if (p->has_error == 0) {
      p->has_error = 1;
      printf("error: missing statement in while body.\n");
    } else {
      printf("error: invalid statement in while body.\n");
    }
    freeASTNode(expr);
    return NULL;
  }

  // no declaration allowed as the directy then body
  if (body_stmt->kind == AST_DECLARATION) {
    p->has_error = 1;
    printf("error: direct declaration is not allowed as body; use a block\n");
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
    printf("error: expected '(' after if statement\n");
    return NULL;
  }

  struct ASTNode * expr = parse_expression(p);
  if (expr == NULL) {
    p->has_error = 1;
    printf("error: missing expression in if statement\n");
    return NULL;
  }

  parse_rparen(p); // This consumes ')', error otherwise
  if ( p->has_error ) {
    printf("error: missing closing parenthesis in if statement\n");
    freeASTNode(expr);
   return NULL;
  }

  // now parse statements. Empty statement are not allowed
  struct ASTNode * then_stmt = parse_statement(p);
  if (then_stmt == NULL) {
    p->has_error = 1;
    printf("error: missing then statement after if condition.\n");
    freeASTNode(expr);
    return NULL;
  }

  // no declaration allowed as the directy then body
  if (then_stmt->kind == AST_DECLARATION) {
    printf("error: direct declaration is not allowed as then-body; use a block\n");
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
      printf("error: expected statement after else\n");
      p->has_error = 1;
      freeASTNode(expr);
      freeASTNode(then_stmt);
      return NULL;
    }

    if (else_stmt->kind == AST_DECLARATION) {
      printf("error: direct declaration is not allowed as else-body; use a block\n");
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


// statement -> declaration | print | block | if | while | break | continue | for_stmt | assignment
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
  } else if (is_current_token(p, TOKEN_WHILE)) {
    node = parse_while_stmt(p);
  } else if (is_current_token(p, TOKEN_FOR)) {
    node = parse_for_stmt(p);
  } else if (is_current_token(p, TOKEN_BREAK)) {
    node = parse_break_stmt(p);
  } else if (is_current_token(p, TOKEN_CONTINUE)) {
    node = parse_continue_stmt(p);
  } else if (is_current_token(p, TOKEN_IDENTIFIER)) {
    if (((p->pos + 1) < BUFFER_SIZE) && is_next_token(p, TOKEN_EQUAL)) {
      node = parse_assignment(p);
    } else if (p->has_error == 0) {
      struct Token bad_token = current_token(p);
      printf("error: found unexpected symbol '%.*s'\n", bad_token.length, bad_token.start);
      p->has_error = 1;
      advance_token(p);
      return NULL;
    }
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
      if (p->has_error == 0) {
        printf("error: expected statement.\n");
        p->has_error = 1;
      }
      break;
    }

    // add ASTNode representing a statement to the list (program node)
    int ret = add_statement_to_program(program, statement);
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
  struct Token tokens[BUFFER_SIZE];
  int token_count = 0;

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
  initialize_symbol_table(&semantic_table, NULL);
  print_symbol_table(&semantic_table);

  struct SemanticContext semantic_ctx;
  semantic_ctx.current_loop = NULL;

  result = analyze_program(program, &semantic_table, &semantic_ctx);

  if (result == FAIL) {
    printf("--> SEMANTIC CHECK PHASE: FAILED\n");
    freeSymbolTableContents(&semantic_table);
    freeASTNode(program);
    free(icode);
    exit(-1);
  }

  printf("Global symbol table after semantic phase\n");
  //print_symbol_table(&semantic_table);
  printf("\nPrint AST Tree after SEMANTIC CHECK PHASE:\n ");
  //print_ast_tree(program, 0);
  freeSymbolTableContents(&semantic_table);
  printf("--> SEMANTIC CHECK PHASE: PASS\n");

  // Program is semantically okay so go ahead and evaluate it
  printf("\n\n--> 4. EVALUATION CHECK PHASE: Start\n");
  printf("Initialize symbol table for Evaluation check\n");
  struct SymbolTable runtime_table;
  initialize_symbol_table(&runtime_table, NULL);
  //print_symbol_table(&runtime_table);
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
