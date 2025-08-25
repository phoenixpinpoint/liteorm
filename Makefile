test:
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; cd tests/bin; ./test

leak_test:
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; cd tests/bin; valgrind --leak-check=full ./test

debug_test: 
	mkdir tests/bin/; cd tests/bin; touch .env; echo "LITEORM_DB=test.db" >> .env;
	gcc -g ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; cd tests/bin; gdb ./test

clean:
	rm -rf ./bin
	rm -rf ./tests/bin
