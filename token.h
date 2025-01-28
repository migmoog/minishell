#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef struct cons {
  char *token;
  struct cons *next;
} cons_t;

/**
 * Returns true if c is a:
 * - ";"
 * - "|"
 * - "<"
 * - ">"
 * - "(" or ")"
 */
int is_special_token(char c);

/**
 * Receive a list of tokens.
 */
cons_t *tokenize(char *str);

/**
 * Count the number of tokens in a list.
 */
const int token_count(cons_t *list); 

/**
 * Free the list of tokens from memory.
 */
void delete_list(cons_t *list);

/**
 * Prints the list of tokens to the console.
 */
void print_list(cons_t *list);

/**
 * Takes a linked list of tokens and flattens them into an array.
 * WARNING: this does not duplicate the strings. They point to the exact same ones as in the list.
 */
void fill_array(char *arr[], cons_t *list);

#endif