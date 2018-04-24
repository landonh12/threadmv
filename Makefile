CC=gcc

threadmake:
	$(CC) -o libmv.o -c libmv.c
	$(CC) -o threadmv threadmv.c libmv.o -g -pthread
