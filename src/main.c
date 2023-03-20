/*
https://stackoverflow.com/questions/31673065/creating-classes-in-c-on-the-stack-vs-the-heap
General rules regarding structs.

Opaque structs = structs that disallow the user to peek into its internals.
Transparent structs = structs that allow the user to peek into its internals.

--- Creation ---

All Opaque structs are defined like this:

    struct _ComplexStruct {....};
    typedef struct _ComplexStruct* ComplexStruct;

All Opaque structs are created like this:

    ComplexStruct data = make_complex_struct();

`make_complex_struct` is responsible for allocating space for the struct itself.

Transparent structs can just be created on the stack:

    struct TransparentStruct data = make_transparent_struct();

--- Deletion ---

All opaque structs are destroyed like this:

    free(data);

`free` is responsible for freeing up the data `data` is pointing to, but not
responsible for setting the pointer to NULL.

--- Usage ---

All opauqe structs are passed by pointer, all transparent structs are passed by
value, unless they require to be passed by pointer (e.g. when change needs to be
made to them).
*/

/*
TODO
- check memory allocations for leaks
- add comment support to the parser (;)
- when a failure happens, do a cleanup
  (check every code block with `if (error_code != SUCCESS)`)
- Add buffered allocation to Env
- Check if the input expression is well-formed (all parens match)
*/

#include "main.h"

struct _Logger _logger;

struct _Parser _create_parser(char* filename);

#ifdef REPL_ENABLED
ErrorCode repl() {
    char buffer[STRING_BUFFER_SIZE];

    int go_on = 1;
    while (go_on) {
        printf("RSC> ");

        if (!fgets(buffer, STRING_BUFFER_SIZE, stdin)) {
            printf("Error while reading from stdin.\n");
        }

        if (buffer[0] == '\n') {
            go_on = 0;
            break;
        }

        Expr expr;
        struct Dict dict;
        uint8_t error_code = parse_from_str(buffer, &expr, &dict);
        if (error_code != SUCCESS) {
            return error_code;
        }
        // print_expr(expr, dict);
        free_expr(&expr);
        expr = NULL;
        printf("\n\n");
    }

    return SUCCESS;
}
#endif

ErrorCode interpret_file(char* file_name) {
    printf("%s\n", file_name);
    struct Dict dict;
    Expr expr;
    uint8_t error_code = parse_from_file(file_name, &expr, &dict);
    if (error_code != SUCCESS) {
        return error_code;
    }
    // print_expr(expr, dict);
    free_expr(&expr);
    expr = NULL;
    free_dict(&dict);
    return SUCCESS;
}

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("----------\nRealScript\n----------\n\n");
    init_logger();
    
    if (argc > 1) {
        // There is at least one parameter
        // interpret_file(argv[1]);

        run_tests();
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

