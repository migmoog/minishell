#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "token.h"

int is_special_token(char c)
{
  switch (c)
  { // check if it's a single character token
  case '<':
  case '>':
  case '|':
  case ';':
  case '(':
  case ')':
    return 1;
  }

  return 0;
}

void fill_array_help(char *arr[], unsigned int i, cons_t *list)
{
  if (list == NULL)
  {
    return;
  }

  arr[i] = list->token;
  fill_array_help(arr, i + 1, list->next);
}

void fill_array(char *arr[], cons_t *list)
{
  fill_array_help(arr, 0, list);
}

void print_list(cons_t *list)
{
  if (list == NULL)
  {
    return;
  }

  puts(list->token);
  print_list(list->next);
}

void delete_list(cons_t *list)
{
  if (list == NULL)
  {
    return;
  }

  free(list->token);
  delete_list(list->next);

  free(list);
}

const int token_count(cons_t *list)
{
  if (list == NULL)
  {
    return 0;
  }

  return 1 + token_count(list->next);
}

#define CHAR_BOUNDS(lower, c, upper) (c <= upper && c >= lower)

// look at ascii chart for what we consider good words
int is_nonspecial(char c)
{
  return CHAR_BOUNDS('0', c, '9') ||
         CHAR_BOUNDS('A', c, 'Z') ||
         CHAR_BOUNDS('a', c, 'z') ||
         c == '_' || c == '-' || c == '.' || c == '/'; // important for files and flags
}

cons_t *tokenize_help(char *str, int i)
{
  if (str[i] == '\0')
  {
    return NULL;
  }

  char *token = NULL;
  while (str[i] != '\0' && token == NULL)
  {
    if (is_special_token(str[i]))
    {
      token = calloc(2, sizeof(char));
      token[0] = str[i];
      token[1] = '\0';
    }

    // found a sentence
    if (str[i] == '\"')
    {
      int len = 0;
      const int sentence_start = i + 1;
      do
      {
        len++;
        i++;
      } while (str[i] != '\"' && str[i] != '\0');
      token = calloc(len, sizeof(char));
      token = memcpy(token, str + sentence_start, len - 1);
      // VERY IMPORTANT: skip ahead of last quote to avoid an empty token!
      if (str[i] == '\"')
      {
        i++;
      }
      break;
    }

    // found a word
    if (is_nonspecial(str[i]))
    {
      const int word_start = i;
      int len = 0;
      while (is_nonspecial(str[i]))
      {
        len++;
        i++;
      }
      token = calloc(len, sizeof(char));
      token = memcpy(token, str + word_start, len);

      break;
    }

    i++;
  }

  if (token == NULL)
  {
    return NULL;
  }
  cons_t *out = malloc(sizeof(cons_t));
  out->token = token;
  out->next = tokenize_help(str, i);
  return out;
}

cons_t *tokenize(char *str) { return tokenize_help(str, 0); }