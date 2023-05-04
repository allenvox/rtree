all: main
.PHONY: main
main: main.c rtree.c
	gcc -Wall -Wextra -o $@ $^

.PHONY: clean
clean:
	rm -rf main