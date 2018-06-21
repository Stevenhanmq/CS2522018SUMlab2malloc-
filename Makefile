
CC = gcc -g -pthread -Wall -Werror
TESTS = test0 test1 test1-1 test1-2 test1-3 test1-4 test2 test3 test4 test5 test6 test7 test8-1 test8-2 test8-3 test8-4 test8-5 test8-7

all: git MyMalloc.so tests

MyMalloc.so: MyMalloc.c
	$(CC) -fPIC -c -g MyMalloc.c
	gcc -shared -o MyMalloc.so MyMalloc.o

tests: $(TESTS)

tests32: $(TESTS:=_32)

test%: test%.c MyMalloc.c
	$(CC) -o $@ $^

test%_32: test%.c MyMalloc.c
	$(CC) -m32 -o $@ $^

git:
	git checkout master >> .local.git.out || echo
	git add *.c *.h  >> .local.git.out || echo
	git commit -a -m "Commit lab 2" >> .local.git.out || echo
	git push origin master

clean:
	rm -f *.o MyMalloc.so $(TESTS) $(TESTS:=_32) core a.out *.out *.txt

cleantests:
	rm -f $(TESTS)

cleantests32:
	rm -f $(TESTS:=_32)
