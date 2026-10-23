#ifndef BOOLE_H
# define BOOLE_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* ---------------------------------------------------------------------------
** Shared AST (src/ast.c): propositional formulas in reverse polish notation.
** ------------------------------------------------------------------------ */

typedef enum e_type
{
	N_CONST,	/* sym = '0' or '1' */
	N_VAR,		/* sym = 'A'..'Z' */
	N_NOT,		/* sym = '!', operand in l */
	N_BIN		/* sym = one of & | ^ > = , operands in l and r */
}	t_type;

typedef struct s_node
{
	t_type			type;
	char			sym;
	struct s_node	*l;
	struct s_node	*r;
}	t_node;

void	*xmalloc(size_t size);
t_node	*ast_new(t_type type, char sym, t_node *l, t_node *r);
void	ast_free(t_node *n);
/* Parses an RPN formula. On error prints a message on stderr, returns NULL. */
t_node	*ast_parse(const char *formula);
bool	ast_op(char op, bool a, bool b);
/* vals[0..25] = value of A..Z (0, 1 or -1 = unbound).
** Returns 0 or 1, or -1 if an unbound variable is reached. */
int		ast_eval(const t_node *n, const signed char *vals);
/* Bit i of the result is set if the variable 'A' + i appears in the tree. */
uint32_t	ast_vars(const t_node *n);
/* Malloc'ed RPN string, to be freed by the caller. */
char	*ast_to_rpn(const t_node *n);

/* ---------------------------------------------------------------------------
** Exercises
** ------------------------------------------------------------------------ */

uint32_t	adder(uint32_t a, uint32_t b);
uint32_t	multiplier(uint32_t a, uint32_t b);
uint32_t	gray_code(uint32_t n);
bool		eval_formula(const char *formula);
void		print_truth_table(const char *formula);
/* NNF of n (or of !n when neg) as a new tree; used by ex05 and ex06. */
t_node		*nnf_node(const t_node *n, bool neg);
/* Return a malloc'ed string, or NULL if the formula is invalid. */
char		*negation_normal_form(const char *formula);
char		*conjunctive_normal_form(const char *formula);
bool		sat(const char *formula);

typedef struct s_powerset
{
	int		**subsets;	/* subsets[i] has sizes[i] elements */
	size_t	*sizes;
	size_t	count;		/* 2^n subsets */
}	t_powerset;

/* NULL if n is too large (n > 30). */
t_powerset	*powerset(const int *set, size_t n);
void		powerset_free(t_powerset *p);

/* sets[i] has sizes[i] elements; set A is sets[0], B is sets[1], ...
** On success returns true and stores a malloc'ed array (sorted) in *out. */
bool		eval_set(const char *formula, const int *const *sets,
				const size_t *sizes, size_t nsets, int **out, size_t *out_size);

double		map(uint16_t x, uint16_t y);
/* Returns false (and prints an error) if n is outside [0, 1). */
bool		reverse_map(double n, uint16_t *x, uint16_t *y);

#endif
