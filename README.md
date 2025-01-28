# Project 1: Shell

A miniature shell implemented with C and Linux syscalls. The main components involved include:
 - a tokenizer to read shell syntax in `token.c/.h`
 - The actual shell implementation in `shell.c`

The [Makefile](Makefile) contains the following targets:

- `make all` - compile everything
- `make tokenize` - compile the tokenizer demo
- `make tokenize-tests` - compile the tokenizer demo
- `make shell` - compile the shell
- `make shell-tests` - run a few tests against the shell
- `make test` - compile and run all the tests
- `make clean` - perform a minimal clean-up of the source tree
