CC=gcc
CFLAGS= -Wall -Werror
DEBUG_FLAGS= -g
RELEASE_FLAGS= -O3
LIBS= -lCBoil

default: test-match test-cleanup

test-match:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_match.c $(LIBS)
	@sudo chmod 0755 test
	@./test
	@rm test

test-match-mem:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_match.c $(LIBS)
	@sudo chmod 0755 test
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./test
	@rm test

test-cleanup:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_cleanup.c $(LIBS)
	@sudo chmod 0755 test
	@./test
	@rm test

test-cleanup-mem:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_cleanup.c $(LIBS)
	@sudo chmod 0755 test
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./test
	@rm test

test-mem: test-match-mem test-cleanup-mem

test-json:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_JSON.c -Wno-maybe-uninitialized $(LIBS) -lCSON -DCSONDEP
	@sudo chmod 0755 test
	@./test
	@rm test

test-json-mem:
	@$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o test test_JSON.c -Wno-maybe-uninitialized $(LIBS) -lCSON -DCSONDEP
	@sudo chmod 0755 test
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./test
	@rm test

json: test-json
json-mem: test-json-mem

clean:
	@rm -f test