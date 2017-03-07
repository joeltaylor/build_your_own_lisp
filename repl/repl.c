#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

/* Declare a buffer for user input of size 2048 */
int main(int argc, char** argcv) {

  /* Print Version and Exit information */
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to exit\n");

  while(1){

    char* input = readline("lipsy> ");

    add_history(input);

    printf("No you're a %s\n", input);

    free(input);
  }

  return 0;

}
