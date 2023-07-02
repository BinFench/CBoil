default: install

static: clean
	@cd gen && python3 gen.py
	@sudo cp -R CBoildef /usr/include/
	@sudo ldconfig
	@cd src && make static --no-print-directory
	@cp src/libCBoil.o libCBoil.o
	@cd src && make clean --no-print-directory

install: clean
	@cd gen && python3 gen.py
	@sudo cp -R CBoildef /usr/include/
	@sudo ldconfig
	@cd src && make install --no-print-directory
	@rm -rf CBoildef

install-quiet: clean
	@make install --no-print-directory > /dev/null

uninstall: clean
	@cd src && make uninstall --no-print-directory

test: install-quiet
	@cd test && make --no-print-directory

make test-mem: install-quiet
	@cd test && make test-mem --no-print-directory

clean:
	@rm -rf -f CBoildef
	@rm -f libCBoil.o
	@cd src && make clean --no-print-directory
	@cd test && make clean --no-print-directory