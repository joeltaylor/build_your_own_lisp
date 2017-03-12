#include "mpc.h"

#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>


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
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
  Number, Operator, Expr, Lispy);

 
    
  /* Print Version and Exit information */
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to exit\n");

  while(1){

    char* input = readline("lipsy> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)){
      /* On success print the AST */
      mpc_ast_print(r.output);
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
