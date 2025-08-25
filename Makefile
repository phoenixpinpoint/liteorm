test:
	mkdir tests/bin/; gcc ./tests/test.c -o ./tests/bin/test -Ideps -lsqlite3; ./tests/bin/test

clean:
	rm -rf ./bin
	rm -rf ./tests/bin
