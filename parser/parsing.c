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

long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  if (strcmp(op, "^") == 0) { return pow(x, y); }
  if (strcmp(op, "%") == 0) { return x % y; }
  return 0;
}

long eval(mpc_ast_t* t) {

  /* If tagged as a number return it directly */
  if(strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  /* The operator is always the second child */
  char* op = t->children[1]->contents;

  /* We store the third child in 'x' */
  long x = eval(t->children[2]);

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
    operator : '+' | '-' | '*' | '/' | '%' | '^' ;                  \
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
      long result = eval(r.output);
      printf("Answer: %li\n", result);
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
