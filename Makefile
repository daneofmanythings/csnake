bin_path = ./bin
target_name = snek
compiler_flags = -lpthread -lncurses -o $(bin_path)/snek


build:
	@ cc main.c $(compiler_flags)

run: build
	@ $(bin_path)/$(target_name)

clean:
	@ rm $(bin_path)/*

.PHONY: build run clean
