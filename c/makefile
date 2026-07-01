
CFLAGS = -std=c23 -Wall -Werror -O2
DEBUG_FLAGS = -fsanitize=address -g
RELEASE_FLAGS = -s

build:
	clang $(CFLAG) $(DEBUG_FLAGS) addsubs.c -o addsubs
	
release: build
	clang $(CFLAGS) $(RELEASE_FLAGS) addsubs.c -o addsubs
	strip addsubs

expand:
	gcc -E addsubs.c > addsubs_expanded.c

test_no_args: build
	./addsubs

test_bag_args: build
	/addsubs . . . .

test_lang: build
	./addsubs . . . jpn
