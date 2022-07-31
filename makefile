SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

OUT = ./bin/app

CC = gcc
CC_VER = c99
CC_FLAGS = -O3 -std=$(CC_VER) -Wall -Wextra -Werror \
           -pedantic -Wno-deprecated-declarations

compile: ./bin $(OBJ) $(SRC)
	$(CC) $(CC_FLAGS) -o $(OUT) $(OBJ) $(CC_LIBS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CC_FLAGS) -o $@

./bin:
	mkdir -p bin

example:
	./bin/app ./examples/hello_world.tmasm
	./bin/app ./examples/loop_to_9.tmasm
	./bin/app ./examples/int_to_str.tmasm

install:
	cp ./bin/app /usr/bin/tmasm

clean:
	rm -r ./bin/*

all:
	@echo compile, example, install, clean
