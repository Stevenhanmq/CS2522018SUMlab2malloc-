
CC = gcc -g -Wall -Werror

all: git MyMalloc.so test0

MyMalloc.so: MyMalloc.c
	$(CC) -fPIC -c -g MyMalloc.c
	gcc -shared -o MyMalloc.so MyMalloc.o

test0: test0.c MyMalloc.c
	$(CC) -o test0 test0.c MyMalloc.c

git:
	git checkout master >> .local.git.out || echo
	git add *.c *.h  >> .local.git.out || echo
	git commit -a -m "Commit lab 2" >> .local.git.out || echo
	git push origin master

clean:
	rm -f *.o MyMalloc.so test0 core a.out *.out *.txt
