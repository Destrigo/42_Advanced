#ifndef BOOLE_H
# define BOOLE_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* ---------------------------------------------------------------------------
** Shared RPN helpers (src/rpn.c)
**
** Formulas are never turned into a recursive tree: they are validated once
** and then evaluated with an explicit stack, so the length of the input is
** only limited by memory and every pass is O(n) with no recursion.
** ------------------------------------------------------------------------ */

void		*xmalloc(size_t size);
void		*xcalloc(size_t count, size_t size);
void		*xrealloc(void *ptr, size_t size);

/* Syntax check of an RPN formula (0 1 A..Z ! & | ^ > =).
** On error prints a message on stderr and returns false. */
bool		rpn_check(const char *f);
bool		rpn_op(char op, bool a, bool b);
/* Bit i of the result is set if the variable 'A' + i appears. f must be valid. */
uint32_t	rpn_vars(const char *f);
/* vals[0..25] = value of A..Z (0, 1 or -1 = unbound); stack needs strlen(f)
** bytes. Returns 0 or 1, or -1 if an unbound variable was used. f must be
** valid. */
int			rpn_eval(const char *f, const signed char *vals, signed char *stack);
/* Same, but evaluates 64 assignments at once: bit j of vals[i] is the value
** of variable i in assignment j. stack needs strlen(f) words. */
uint64_t	rpn_eval_word(const char *f, const uint64_t *vals, uint64_t *stack);

/* ---------------------------------------------------------------------------
** Exercises
** ------------------------------------------------------------------------ */

uint32_t	adder(uint32_t a, uint32_t b);
uint32_t	multiplier(uint32_t a, uint32_t b);
uint32_t	gray_code(uint32_t n);
bool		eval_formula(const char *formula);
void		print_truth_table(const char *formula);
/* Return a malloc'ed string, or NULL if the formula is invalid or the result
** would be absurdly large (NNF/CNF can grow exponentially). */
char		*negation_normal_form(const char *formula);
char		*conjunctive_normal_form(const char *formula);
bool		sat(const char *formula);

typedef struct s_powerset
{
	int		**subsets;	/* subsets[i] has sizes[i] elements */
	size_t	*sizes;
	size_t	count;		/* 2^n subsets */
	int		*data;		/* backing storage of all the subsets */
}	t_powerset;

/* NULL (with a message) if the result would not fit in memory. */
t_powerset	*powerset(const int *set, size_t n);
void		powerset_free(t_powerset *p);

/* sets[i] has sizes[i] elements; set A is sets[0], B is sets[1], ...
** On success returns true and stores a malloc'ed array (sorted) in *out. */
bool		eval_set(const char *formula, const int *const *sets,
				const size_t *sizes, size_t nsets, int **out, size_t *out_size);

double		map(uint16_t x, uint16_t y);
/* Returns false (and prints an error) if n is outside [0, 1); x and y are
** set to 0 in that case. */
bool		reverse_map(double n, uint16_t *x, uint16_t *y);

#endif
