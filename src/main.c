// TODO
// - check memory allocations for leaks

#define REPL_ENABLED // Comment this out to disable the repl
#define BUFFER_SIZE 1024

#include <stdio.h>
#include <stdlib.h>

#include "parse.h"

struct Parser create_parser(char* filename);

#ifdef REPL_ENABLED
void repl() {
    char buffer[BUFFER_SIZE];

    int go_on = 1;
    while (go_on) {
        printf("REAL: ");

        if (!fgets(buffer, BUFFER_SIZE, stdin)) {
            printf("Error while reading from stdin.\n");
        }

        if (buffer[0] == '\n') {
            go_on = 0;
            break;
        }

        struct Expr* expr = parse_from_str(buffer);
        print_expr(expr);
        destroy_expr(expr);
        printf("\n\n");
    }

    free(buffer);
}
#endif

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("----------\nRealScript\n----------\n\n");
    if (argc > 1) {
        // There is at least one parameter
        char* filename = argv[1];
        printf("%s\n", filename);
        struct Expr* expr = parse_from_file(filename);
        print_expr(expr);
        destroy_expr(expr);
    } else {
        #ifdef REPL_ENABLED
        // There are no arguments, start a REPL
        repl();
        #else
        printf("Error: no filename provided.\n");
        #endif
    }
    return 0;
}

