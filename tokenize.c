#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "token.h"

int main(int argc, char **argv) {
  char *str;
  // check if we got an arg
  if (argc == 2) {
    str = argv[1];
  } else {
    // otherwise read from stdin
    str = calloc(256, sizeof(char));
    int length = read(0, str, 255);
    str[length] = '\0';
    if (length < 0) {
      perror("read failed");
      free(str);
      exit(1);
    }
  }

  cons_t *list = tokenize(str);

  print_list(list);
  delete_list(list);
  return 0;
}