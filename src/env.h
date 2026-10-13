#ifndef ENV_H
#define ENV_H

#include <stdbool.h>

// L'ambiente: l'insieme delle variabili definite finora (una lista concatenata).
typedef struct Env Env;

Env *env_new(void);
void env_free(Env *e);
bool env_get(const Env *e, const char *name, double *value);
int  env_set(Env *e, const char *name, double value);   // 0 = ok, -1 = memoria esaurita

#endif
