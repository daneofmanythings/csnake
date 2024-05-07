bin_path = ./bin
source_files = gamedata.c

target_name = snek
compiler_flags = -lpthread -lncurses -o $(bin_path)/$(target_name)

test_target_name = test
test_compiler_flags = -lpthread -lncurses -lcriterion -o $(bin_path)/$(test_target_name)


build:
	@ cc main.c $(source_files) $(compiler_flags)

test:
	@ cc test.c $(source_files) $(test_compiler_flags)
	@ $(bin_path)/$(test_target_name)

run: build
	@ $(bin_path)/$(target_name)

clean:
	@ rm $(bin_path)/*

.PHONY: build run clean
