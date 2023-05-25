CC=gcc
CFLAGS= -c -Wall -Werror -fpic
DEBUG_FLAGS= -g
OBJS=CBoil.o CBoil/printutils.o
SHARED=libCBoil.so
SHARED_FLAGS= -shared -o
INC=/usr/include
LIB=/usr/lib

default: install

install: $(OBJS)
	@ld -r -o libCBoil.o $(OBJS)
	@$(CC) $(SHARED_FLAGS) $(SHARED) libCBoil.o
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
	$(CC) -c -o $@ $< $(CFLAGS) $(DEBUG_FLAGS)

CBoil/printutils.o: CBoil/printutils.c
	$(CC) -c -o $@ $< -Wno-return-local-addr $(CFLAGS) $(DEBUG_FLAGS)