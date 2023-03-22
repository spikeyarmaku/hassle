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
- add Unicode support
- check memory allocations for leaks
- when a failure happens, do a cleanup
  (check every code block with `if (error_code != Success)`)
- Check if the input expression is well-formed (all parens match)
- add `_t` suffix to typedef'd types
- rename size_t to INDEX where it makes sense
- when freeing an object, overwrite the memory with random data
  (SECURE_DESTRUCTION)
*/

#include "main.h"

struct _Logger _logger;

// enum ErrorCode interpret(EnvFrame_t env, Expr_t expr) {
//     struct Term result;
//     enum ErrorCode Error_code = eval_expr(env, expr, &result);
//     if (Error_code != Success) {
//         return Error_code;
//     }
    
//     char buf[1024];
//     print_term(buf, result, env);
//     printf("%s", buf);

//     free_term(result);

//     return Success;
// }

#ifdef REPL_ENABLED
enum ErrorCode repl() {
    // char buffer[STRING_BUFFER_SIZE];

    // int go_on = 1;
    // EnvFrame_t env = make_default_frame();
    // while (go_on) {
    //     printf("RSC> ");

    //     if (!fgets(buffer, STRING_BUFFER_SIZE, stdin)) {
    //         printf("Error while reading from stdin.\n");
    //     }

    //     if (buffer[0] == '\n') {
    //         go_on = 0;
    //         break;
    //     }

    //     ErrorCode_t error_code;
    //     Expr_t expr = parse_from_str(&error_code, buffer);
    //     if (error_code != Success) {
    //         return error_code;
    //     }
        
    //     interpret(env, expr);
    //     free_expr(&expr);
    // }

    // free_frame(&env);
    return Success;
}
#endif

ErrorCode_t interpret_file(char* file_name) {
    printf("%s\n", file_name);
    // EnvFrame_t env = make_default_frame();
    ErrorCode_t error_code;
    Expr_t expr = parse_from_file(&error_code, file_name);
    if (error_code != Success) return error_code;

    char* str = expr_to_string(expr);
    printf("Expression: %s\n", str);
    free_mem("main", str);
    // interpret(env, expr);
    // free_env(&env);
    expr_free(&expr);

    show_logger_entries(_logger);
    return Success;
}


struct Test{
    char* msg;
};
struct Test char_test(char* msg) {
    struct Test t;
    t.msg = msg;
    return t;
}

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("----------\nRealScript\n----------\n\n");
    init_logger();
    
    if (argc > 1) {
        // There is at least one parameter
        ErrorCode_t error_code = interpret_file(argv[1]);
        if (error_code != Success) return 1;

        // run_tests();
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

