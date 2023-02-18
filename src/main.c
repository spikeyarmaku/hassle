// TODO
// - check memory allocations for leaks

#include <stdio.h>
#include <stdlib.h>

#include "parse.h"

#define BUFFER_SIZE 1024

struct Parser create_parser(char* filename);

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("----------\nRealScript\n----------\n\n");
    if (argc > 1) {
        // There is at least one parameter
        char* filename = argv[1];
        printf("%s\n", filename);
        struct Expr* expr = parse(filename);
        print_expr(expr);
        destroy_expr(expr);
        printf("\nCLEAN\n");
    } else {
        // There are no arguments, start a REPL
        printf("REPL TIME");
    }
    return 0;
}

