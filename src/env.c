#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "lexer.h"

typedef struct Var {
    char        name[NAME_MAX_LEN];
    double      value;
    struct Var *next;
} Var;

struct Env {
    Var *vars;
};

Env *env_new(void) {
    return calloc(1, sizeof(Env));
}

void env_free(Env *e) {
    if (e == NULL) {
        return;
    }
    Var *v = e->vars;
    while (v != NULL) {
        Var *next = v->next;
        free(v);
        v = next;
    }
    free(e);
}

static Var *find(const Env *e, const char *name) {
    for (Var *v = e->vars; v != NULL; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            return v;
        }
    }
    return NULL;
}

bool env_get(const Env *e, const char *name, double *value) {
    Var *v = find(e, name);
    if (v == NULL) {
        return false;
    }
    *value = v->value;
    return true;
}

int env_set(Env *e, const char *name, double value) {
    Var *v = find(e, name);
    if (v == NULL) {
        v = malloc(sizeof *v);
        if (v == NULL) {
            return -1;
        }
        strncpy(v->name, name, sizeof v->name - 1);
        v->name[sizeof v->name - 1] = '\0';
        v->next = e->vars;              // inserimento in testa
        e->vars = v;
    }
    v->value = value;
    return 0;
}
