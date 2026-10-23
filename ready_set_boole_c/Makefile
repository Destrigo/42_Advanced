CC       = cc
CFLAGS   = -Wall -Wextra -Werror -std=c99 -pedantic -O2 -Iinclude

ifeq ($(OS),Windows_NT)
EXE      = .exe
endif

SRC      = src/ast.c src/ex00_adder.c src/ex01_multiplier.c \
           src/ex02_gray_code.c src/ex03_eval_formula.c \
           src/ex04_truth_table.c src/ex05_nnf.c src/ex06_cnf.c \
           src/ex07_sat.c src/ex08_powerset.c src/ex09_eval_set.c \
           src/ex10_curve.c src/ex11_inverse.c
OBJ      = $(SRC:src/%.c=obj/%.o)
NAMES    = ex00 ex01 ex02 ex03 ex04 ex05 ex06 ex07 ex08 ex09 ex10 ex11
BINS     = $(NAMES:%=bin/%$(EXE))

all: $(BINS)

obj/%.o: src/%.c include/boole.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

bin/%$(EXE): mains/%_main.c $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $< $(OBJ) -o $@

bin/tests$(EXE): tests/tests.c $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $< $(OBJ) -o $@

test: bin/tests$(EXE)
	./bin/tests$(EXE)

clean:
	rm -rf obj

fclean: clean
	rm -rf bin

re: fclean all

.PHONY: all test clean fclean re
