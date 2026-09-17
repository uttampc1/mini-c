#include <stdio.h>
enum TokenType {
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_PLUS,
  TOKEN_STAR,
  TOKEN_EQUAL,
  TOKEN_SEMICOLON,
  TOKEN_UNKNOWN,
  TOKEN_EOF
};

struct Token {
  enum TokenType type;
  char *start;
  int length;
};

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
    case TOKEN_EQUAL:
      return "EQUAL";
    case TOKEN_SEMICOLON:
      return "SEMICOLON";
    case TOKEN_UNKNOWN:
      return "UNKNOWN";
    case TOKEN_EOF:
      return "EOF";
  }
  return "UNKNOWN";
}

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

// code="42;"
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
    } else if (is_semicolon(code[idx])) {
      add_token(tokens, token_count, TOKEN_SEMICOLON, code + idx, 1);
      idx++;
    } else if (is_star(code[idx])) {
      add_token(tokens, token_count, TOKEN_STAR, code + idx, 1);
      idx++;
    } else if (is_equal(code[idx])) {
      add_token(tokens, token_count, TOKEN_EQUAL, code + idx, 1);
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

int main(void) {
  char *code = "x=123+45*2;";
  struct Token tokens[100];
  int token_count = 0;
  printf("Input: %s\n", code);
  tokenize(code, tokens, &token_count);
  for (int i = 0; i < token_count; i++) {
    print_token(tokens[i]);
  }
#if 0
  if (is_plus(code[idx])) {
    printf("Found plus sign\n");
    idx++;
  }
  skip_spaces(code, &idx);
  printf("New idx: %d, Next Char:%c\n", idx, code[idx]);
  scan_number(code, idx);
#endif
  return 0;
}
