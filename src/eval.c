#include <stdio.h>
#include "eval.h"

bool eval(const Node *n, Env *env, double *out, char *err, size_t err_size) {
    double a, b;

    switch (n->kind) {
        case N_NUM:
            *out = n->as.num;
            return true;

        case N_VAR:
            if (!env_get(env, n->as.name, out)) {
                snprintf(err, err_size, "variabile '%s' non definita", n->as.name);
                return false;
            }
            return true;

        case N_NEG:
            if (!eval(n->as.neg.operand, env, &a, err, err_size)) {
                return false;
            }
            *out = -a;
            return true;

        case N_BIN:
            if (!eval(n->as.bin.l, env, &a, err, err_size) ||
                !eval(n->as.bin.r, env, &b, err, err_size)) {
                return false;
            }
            switch (n->as.bin.op) {
                case '+': *out = a + b; return true;
                case '-': *out = a - b; return true;
                case '*': *out = a * b; return true;
                case '/':
                    if (b == 0) {
                        snprintf(err, err_size, "divisione per zero");
                        return false;
                    }
                    *out = a / b;
                    return true;
            }
            return false;

        case N_ASSIGN:
            if (!eval(n->as.assign.value, env, &a, err, err_size)) {
                return false;
            }
            if (env_set(env, n->as.assign.name, a) != 0) {
                snprintf(err, err_size, "memoria esaurita");
                return false;
            }
            *out = a;
            return true;
    }
    return false;
}
