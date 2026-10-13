#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include <stddef.h>
#include "ast.h"
#include "env.h"

// Calcola il valore dell'albero. Se fallisce restituisce false e scrive il motivo in err.
bool eval(const Node *n, Env *env, double *out, char *err, size_t err_size);

#endif
