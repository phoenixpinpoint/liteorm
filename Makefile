build:
	mkdir bin;
	gcc -c -fPIC src/liteorm.c -o bin/liteorm.o -Ideps -lsqlite3
	gcc bin/liteorm.o -shared -o bin/libliteorm.so

install: 
	cp bin/libliteorm.so /usr/lib
	cp src/liteorm.h /usr/include

uninstall:
	rm -rf /usr/lib/libliteorm.so
	rm -rf /usr/include/liteorm.h

test:
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; cd tests/bin; ./test

test_install:
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3 -lliteorm; cd tests/bin; ./test 

test_leak:
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; cd tests/bin; valgrind --leak-check=full ./test

test_debug: 
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; cd tests/bin; gdb ./test

clean:
	rm -rf ./bin
	rm -rf ./tests/bin
