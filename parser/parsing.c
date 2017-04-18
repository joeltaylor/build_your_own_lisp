#include "mpc.h"

#include <stdio.h>
#include <stdlib.h>

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

#else
#include <editline/readline.h>
#endif

int number_of_nodes(mpc_ast_t* t) {
  if(t->children_num == 0) {return 1;}
  if(t->children_num >= 1){
    int total = 1;
    for(int i = 0; i < t->children_num; i++){
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}

int number_of_leaves(mpc_ast_t* t) {
  if(t->children_num == 0) { return 1; }
  if(t->children_num >= 1) {
    int total = 0;
    for(int i = 0; i < t->children_num; i++){
      total = total + number_of_leaves(t->children[i]);
    }
    return total;
  }
  return 0;
}

int number_of_branches(mpc_ast_t* t){
  if(t->children_num == 0) { return 0; }
  if(t->children_num >= 1) {
    int total = 1;
    for(int i = 0; i < t->children_num; i++){
      total = total + number_of_branches(t->children[i]);
    }
    return total;
  }
  return 0;
}

typedef struct {
  int type;
  long num;
  int err;
} lval;

/* Create Enumberation of possible lval types */
enum { LVAL_NUM, LVAL_ERR };

/* Create Enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create a new number type lval */
lval lval_num(long x){
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

lval lval_err(int x){
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

void lval_print(lval v) {
  switch (v.type) {
    case LVAL_NUM: printf("%li", v.num); break;
    case LVAL_ERR: 
      if (v.err == LERR_DIV_ZERO) {
        printf("Error: Division by zero!");
      }
      if (v.err == LERR_BAD_OP) {
        printf("Error: Invalid operator!");
      }
      if (v.err == LERR_BAD_NUM) {
        printf("Error: Invalid number!");
      }
      break;
  }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y) {

  /* If either value is an error return it */
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }
  if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
  if (strcmp(op, "/") == 0) {
    /* If second operand is zero return error */
    return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  }

  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {

  /* If tagged as a number return it directly */
  if(strstr(t->tag, "number")) {
    /* Check if there is some error in conversion */
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always the second child */
  char* op = t->children[1]->contents;

  /* We store the third child in 'x' */
  lval x = eval(t->children[2]);

  /* Iterate over the remaining children and combining */
  int i = 3;
  while(strstr(t->children[i]->tag, "expr")){
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}


/* Declare a buffer for user input of size 2048 */
int main(int argc, char** argcv) {
  /* Create some parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

  /* Define them with the following language */
  mpca_lang(MPCA_LANG_DEFAULT,
  "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' | '%' | '^' ;      \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
  Number, Operator, Expr, Lispy);


  /* Print Version and Exit information */
  puts("Tao Version 0.0.0.0.1");
  puts("Press Ctrl+c to exit\n");

  while(1){

    char* input = readline("tao> ");

    add_history(input);

    /* Attempt to Parse the user Input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)){
      /* Load AST from output */
      mpc_ast_t* a = r.output;

      mpc_ast_print(a);
      printf("Number of nodes: %i\n",  number_of_nodes(a));
      printf("Number of leaves: %i\n",  number_of_leaves(a));
      printf("Number of branches: %i\n",  number_of_branches(a));

      /* On success print the AST */
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    }else{
      /* Otherwise print the error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}
