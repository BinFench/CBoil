CC=gcc
CFLAGS= -Wall -Werror
DEBUG_FLAGS = -g
LIBS= -lCBoil

default:
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o test test_match.c $(LIBS)
	@sudo chmod 0755 test
	@./test
	@rm test

test-mem:
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o test test_match.c $(LIBS)
	@sudo chmod 0755 test
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./test
	@rm test

clean:
	@rm -f test