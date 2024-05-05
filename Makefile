build:
	@ cc main.c -lncurses -o snek

run: build
	@ ./snek
