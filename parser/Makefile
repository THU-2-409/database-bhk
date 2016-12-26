all: a.out

a.out: parser.l parser.y func.h
	flex parser.l
	bison -d parser.y
	g++ parser.tab.c -g

clean:
	rm a.out
