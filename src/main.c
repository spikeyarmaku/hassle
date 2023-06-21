/*
TODO
- add Unicode support
- add locale support (decimal separators, etc.)
- check memory allocations for leaks
- Check if the input expression is well-formed (all parens match)
- add the ability to restrict the VM to a certain word size.
    This is so a vm state can be loaded on a 16-bit machine, even though it was
    saved on a 64-bit machine
*/

/*
Coding convenctions:

- each function is responsible for cleaning up the arguments they receive
  (callee-frees)
*/

#include "main.h"
#include "vm/vm.h"

// enum ErrorCode interpret(Frame_t env, Expr_t expr) {
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
    // Frame_t env = make_default_frame();
    // while (go_on) {
    //     printf("REAL> ");

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

void interpret_file(char* file_name) {
    printf("%s\n", file_name);
    
    debug("\n\n------PARSE------\n\n");

    Expr_t* expr = parse_from_file(file_name);

    VM_t* vm = vm_init(expr);

    debug("\n\n------EVAL------\n\n");

    Term_t* result = vm_run(vm);
    (void)result;

    // if (result != NULL) {
    //     printf("RESULT:\n");
    //     term_print(result);
    //     printf("\n");
    // }

    debug("\n\n------FREE VM------\n\n");

    vm_free(vm);

    show_logger_entries();
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
    setvbuf(stdout, (char *) NULL, _IONBF, 0); /* make stdout line-buffered */
    init_logger();
    
    if (argc > 1) {
        // There is at least one parameter
        
        interpret_file(argv[1]);
        
        // char buf[100];
        // sprintf(buf, "126");
        // Alnat_t alnat1 = string_to_alnat(buf);
        // Alnat_t alnat1copy = alnat_copy(alnat1);
        // sprintf(buf, "2");
        // Alnat_t alnat2 = string_to_alnat(buf);
        // Alnat_t alnat2copy = alnat_copy(alnat2);
        // Alnat_t sum = alnat_add(alnat1copy, alnat2copy);
        // debug("\nResult's raw bytes:\n");
        // alnat_print_raw_bytes(sum);

        // Alnat_t sumcopy = alnat_copy(sum);
        // debug("\nRESULT:\n\n");
        // alnat_print(sumcopy);

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

