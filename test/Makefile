CC=gcc
CFLAGS= -Wall -Werror
DEBUG_FLAGS= -g
RELEASE_FLAGS= -O3
LIBS= -lCBoil

default:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_match.c $(LIBS)
	@sudo chmod 0755 test
	@./test
	@rm test

test-mem:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_match.c $(LIBS)
	@sudo chmod 0755 test
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./test
	@rm test

json:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_JSON.c -Wno-maybe-uninitialized $(LIBS) -lCSON -DCSONDEP
	@sudo chmod 0755 test
	@./test
	@rm test

json-mem:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_JSON.c -Wno-maybe-uninitialized $(LIBS) -lCSON -DCSONDEP
	@sudo chmod 0755 test
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./test
	@rm test

clean:
	@rm -f test