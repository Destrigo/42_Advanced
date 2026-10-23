CC       = cc
# -ffp-contract=off: the compiler must not fuse a * b + c by itself, the only
# fused operations are the explicit fmaf calls the subject allows.
CFLAGS   = -Wall -Wextra -Werror -std=c99 -pedantic -O2 -ffp-contract=off \
           -Iinclude -Isrc
LDLIBS   = -lm

ifeq ($(OS),Windows_NT)
EXE      = .exe
endif

SRC      = src/field.c src/types.c src/elim.c \
           src/ex00_add_sub_scale.c src/ex01_linear_combination.c \
           src/ex02_linear_interpolation.c src/ex03_dot_product.c \
           src/ex04_norm.c src/ex05_cosine.c src/ex06_cross_product.c \
           src/ex07_matrix_multiplication.c src/ex08_trace.c \
           src/ex09_transpose.c src/ex10_row_echelon.c \
           src/ex11_determinant.c src/ex12_inverse.c src/ex13_rank.c \
           src/ex14_projection.c
OBJ      = $(SRC:src/%.c=obj/%.o)
NAMES    = ex00 ex01 ex02 ex03 ex04 ex05 ex06 ex07 ex08 ex09 ex10 ex11 ex12 \
           ex13 ex14 ex15
BINS     = $(NAMES:%=bin/%$(EXE))

all: $(BINS)

obj/%.o: src/%.c include/matrix.h src/internal.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

bin/%$(EXE): mains/%_main.c $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(LDLIBS)

bin/tests$(EXE): tests/tests.c $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(LDLIBS)

test: bin/tests$(EXE)
	./bin/tests$(EXE)

clean:
	rm -rf obj

fclean: clean
	rm -rf bin

re: fclean all

.PHONY: all test clean fclean re
