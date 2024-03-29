CC=gcc
CFLAGS= -c -Wall -Werror -fpic
DEBUG_FLAGS= -g
RELEASE_FLAGS= -O3
OBJS=CBoil.o CBoil/printutils.o
SHARED=libCBoil.so
SHARED_FLAGS= -shared -o
INC=/usr/include
LIB=/usr/lib

default: install

static: $(OBJS)
	@ld -r -o libCBoil.o $(OBJS)

install: static
	@$(CC) $(SHARED_FLAGS) $(SHARED) libCBoil.o
	@sudo cp $(SHARED) $(LIB)
	@sudo cp CBoil.h $(INC)
	@sudo cp CBoil.def $(INC)
	@sudo cp -r CBoil $(INC)
	@sudo chmod 0755 $(LIB)/$(SHARED)
	@sudo ldconfig
	@rm $(OBJS) $(SHARED) libCBoil.o
	@echo "Shared library installed."

install-json:
	@$(CC) -c -o CBoil/printutils.o CBoil/printutils.c -Wno-return-local-addr -DCSONDEP -lCSON $(CFLAGS) $(RELEASE_FLAGS)
	@make static
	@$(CC) $(SHARED_FLAGS) $(SHARED) libCBoil.o
	@make uninstall --no-print-directory > /dev/null
	@sudo cp -R ../CBoildef /usr/include/
	@sudo ldconfig
	@sudo cp $(SHARED) $(LIB)
	@sudo cp CBoil.h $(INC)
	@sudo cp CBoil.def $(INC)
	@sudo cp -r CBoil $(INC)
	@sudo chmod 0755 $(LIB)/$(SHARED)
	@sudo ldconfig
	@rm $(OBJS) $(SHARED) libCBoil.o
	@echo "Shared library installed."

uninstall:
	@sudo rm $(INC)/CBoil.h
	@sudo rm $(INC)/CBoil.def
	@sudo rm -rf $(INC)/CBoildef
	@sudo rm -rf $(INC)/CBoil
	@sudo rm $(LIB)/$(SHARED)
	@sudo ldconfig
	@echo "Shared library uninstalled."

%.o: %.c
	@$(CC) -c -o $@ $< $(CFLAGS) $(RELEASE_FLAGS)

CBoil/printutils.o: CBoil/printutils.c
	@$(CC) -c -o $@ $< -Wno-return-local-addr $(CFLAGS) $(RELEASE_FLAGS)

clean:
	@rm -f CBoil.o
	@rm -f libCBoil.o
	@rm -f CBoil/printutils.o