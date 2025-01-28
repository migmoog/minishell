#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "token.h"
#include "vect.h"

const unsigned int max_input = 255;

void run_cmd(vect_t *v)
{
  const int size = vect_size(v);
  if (size == 0)
    return;

  char *args[size + 1];
  for (int i = 0; i < size; i++)
    args[i] = vect_get(v, i);
  args[size] = NULL;

  pid_t child_pid = fork();
  if (child_pid == 0 && execvp(args[0], args) == -1)
  {
    printf("%s: command not found\n", args[0]);
    assert(fflush(0) == 0);
    exit(1);
  }
}

void redirect(vect_t *v, char *path, int dir)
{
  const int size = vect_size(v);
  char *args[size + 1];
  for (int i = 0; i < size; i++)
    args[i] = vect_get(v, i);
  args[size] = NULL;

  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    close(dir);

    if (dir && open(path, O_CREAT | O_TRUNC | O_APPEND | O_WRONLY, 0644) == -1)
    {
      perror("Failed to open file");
      exit(1);
    }
    else
      open(path, O_RDONLY);

    if (execvp(args[0], args) == -1)
      exit(1);
  }
}

void parse_stream_help(cons_t *l, vect_t *v);
void parse_stream(cons_t *list);

void pipecmd(vect_t *v, cons_t *remaining)
{
  char *in_path = NULL;
  vect_t *good_tokens = vect_new();
  for (int i = 0; i < vect_size(v); i++) {
    const char *elt = vect_get(v, i);
    if (elt[0] == '<') {
      i++;
      in_path = vect_get(v, i);
      i++;
      continue;
    }

    vect_add(good_tokens, vect_get(v, i));
  }

  const int size = vect_size(good_tokens);
  char *args[size + 1];
  for (int i = 0; i < size; i++)
    args[i] = vect_get(good_tokens, i);
  args[size] = NULL;

  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    exit(1);
  }

  // command1 
  pid_t a = fork();
  if (a == 0) {
    close(pipe_fd[0]);
    dup2(pipe_fd[1], STDOUT_FILENO);
    close(pipe_fd[1]);

    if (in_path != NULL) {
      close(0);
      open(in_path, O_RDONLY);
    }

    if (execvp(args[0], args) == -1) {
      perror("execvp");
      exit(1);
    }
  }

  // command2 
  pid_t b = fork();
  if (b == 0) { 
    close(pipe_fd[1]);
    dup2(pipe_fd[0], STDIN_FILENO);
    close(pipe_fd[0]);

    parse_stream(remaining);
    exit(0);
  }

  close(pipe_fd[0]);
  close(pipe_fd[1]);

  waitpid(a, NULL, 0);
  waitpid(b, NULL, 0);

  vect_delete(v);
  vect_delete(good_tokens);
}


void parse_stream(cons_t *list) { parse_stream_help(list, vect_new()); }
void parse_stream_help(cons_t *list, vect_t *token_v)
{
  const int reached_end = list == NULL;
  if (reached_end || list->token[0] == ';')
  {
    run_cmd(token_v);
    wait(NULL);

    if (!(reached_end || list->next == NULL))
    {
      vect_delete(token_v);
      parse_stream(list->next);
    }
  }
  else if ((list->token[0] == '<' 
    && list->next != NULL 
    && (list->next->next == NULL || list->next->next->token[0] == ';')) 
  || list->token[0] == '>')
  {
    redirect(token_v, list->next->token, list->token[0] == '>');
    wait(NULL);
    parse_stream(list->next->next);
  }
  else if (list->token[0] == '|')
  {
    pipecmd(token_v, list->next);
    return;
  }
  else
  {
    vect_add(token_v, list->token);
    parse_stream_help(list->next, token_v);
  }
}

void change_directory(cons_t *list)
{
  // make sure there is an actual argument
  const int cd_argc = token_count(list);
  if (cd_argc < 2)
  {
    puts("No path provided for cd.");
    delete_list(list);
    return;
  }

  if (chdir(list->next->token) == -1)
  {
    puts("Directory not found");
  }

  delete_list(list);
}

void source(cons_t *list)
{
  const int source_argc = token_count(list);
  if (source_argc < 2)
  {
    puts("No file provided for source command.");
    delete_list(list);
    return;
  }

  FILE *source = fopen(list->next->token, "r");
  if (source == NULL)
  {
    puts("Could not find the provided file.");
    return;
  }
  char buf[max_input];
  while (fgets(buf, max_input, source) != NULL)
    parse_stream(tokenize(buf));

  delete_list(list);
}

#define IS_BUILTIN(str) (!strcmp(str, list->token))
int main(int argc, char **argv)
{
  puts("Welcome to mini-shell.");

  char buf[max_input];
  cons_t *prev_list = NULL;
  int called_prev = 0;
  while (1)
  {
    cons_t *list;
    if (!called_prev)
    {
      printf("shell $ ");
      assert(fflush(0) == 0);
      if (fgets(buf, max_input, stdin) == NULL)
        break;
      list = tokenize(buf);
      if (list == NULL)
        continue;
    }
    else
    {
      called_prev = 0;
      list = prev_list;
    }

    if (IS_BUILTIN("exit"))
    {
      puts("Bye bye.");
      break;
    }
    else if (IS_BUILTIN("help"))
    {
      puts(" - cd: change the current working directory.");
      puts(" - source: run a list of commands from a specified file.");
      puts(" - prev: print the previous command and run it");
      puts(" - help: the one you just ran silly!");
    }
    else if (IS_BUILTIN("cd"))
      change_directory(list);
    else if (IS_BUILTIN("source"))
      source(list);
    else if (IS_BUILTIN("prev"))
    {
      if (prev_list == NULL)
      {
        puts("No command has been called yet.");
        continue;
      }
      called_prev = 1;
      continue;
    }
    else
    { // if none of the commands were builtins then try it as a regular command
      parse_stream(list);
    }

    prev_list = list;
  }

  return 0;
}