// calc: un micro-interprete per espressioni aritmetiche con variabili.
//   ./calc            modalità interattiva (REPL)
//   ./calc file.calc  esegue un file, una riga alla volta
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "env.h"
#include "eval.h"
#include "parser.h"

// Esegue una riga. Restituisce 0 se è andata bene, 1 se c'è stato un errore.
static int run_line(const char *line, Env *env, const char *where) {
    ParseError perr;
    Node *tree = parse_line(line, &perr);
    if (tree == NULL) {
        if (perr.message[0] == '\0') {
            return 0;                       // riga vuota o solo commento
        }
        fprintf(stderr, "%s: errore: %s\n  %s\n  %*s^\n",
                where, perr.message, line, (int)perr.pos, "");
        return 1;
    }

    char err[96];
    double value;
    int rc = 0;
    if (eval(tree, env, &value, err, sizeof err)) {
        if (tree->kind == N_ASSIGN) {
            printf("%s = %.10g\n", tree->as.assign.name, value);
        } else {
            printf("%.10g\n", value);
        }
    } else {
        fprintf(stderr, "%s: errore: %s\n", where, err);
        rc = 1;
    }
    node_free(tree);
    return rc;
}

int main(int argc, char *argv[]) {
    FILE *in = stdin;
    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            perror(argv[1]);
            return 1;
        }
    } else if (argc > 2) {
        fprintf(stderr, "uso: %s [file]\n", argv[0]);
        return 2;
    }

    Env *env = env_new();
    if (env == NULL) {
        fprintf(stderr, "memoria esaurita\n");
        return 1;
    }

    int interactive = (in == stdin) && isatty(STDIN_FILENO);
    char line[256];
    int lineno = 0, errors = 0;

    for (;;) {
        if (interactive) {
            printf("> ");
            fflush(stdout);
        }
        if (fgets(line, sizeof line, in) == NULL) {
            break;
        }
        lineno++;

        // fgets si ferma a 255 caratteri: se non c'è il '\n' la riga era più lunga
        if (strchr(line, '\n') == NULL && !feof(in)) {
            int c;
            while ((c = fgetc(in)) != EOF && c != '\n') {
                // scarta il resto della riga
            }
            fprintf(stderr, "riga %d: errore: riga troppo lunga (max %zu caratteri)\n", lineno, sizeof line - 1);
            errors++;
            continue;
        }
        line[strcspn(line, "\n")] = '\0';

        char where[32];
        snprintf(where, sizeof where, interactive ? "riga" : "riga %d", lineno);
        errors += run_line(line, env, where);
    }

    env_free(env);
    if (in != stdin) {
        fclose(in);
    }
    return errors ? 1 : 0;
}
