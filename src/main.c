/*
- add Unicode support
- add locale support (decimal separators, etc.)
- use boehm's GC
- Check if the input expression is well-formed (all parens match)
*/

#include "config.h"
#include "global.h"

#include "parse\parse.h"
#include "serialize\serialize.h"

#include "network.h"
#include "memory.h"

#include "tree/vm.h"
#include "tree/primop.h"

// DEBUG
// #include "tree/eval.h"
#include "tree/combinators.h"

#include "response.h"

struct Config {
    BOOL open_socket;
    uint16_t port;
    BOOL log_memory;
    char* file_to_interpret;
};

typedef struct Config Config_t;

Config_t        _config_init        ();
ErrorCode_t     _flag_handle        (Config_t*, char*);
void            _interpreter_start  (Config_t*);
void            _print_help_message ();
struct Program* _interpret_file     (struct VM*, char*);
void            _repl_start_local   (struct VM*);
void            _repl_start_remote  (struct VM*, Connection_t);
Response_t*     _execute_command    (struct VM*, char*);

Config_t  _config_init() {
    Config_t config; // = (Config_t*)allocate_mem(NULL, NULL, sizeof(Config_t));
    config.open_socket = FALSE;
    config.port = 0;
    config.log_memory = FALSE;
    config.file_to_interpret = NULL;
    return config;
}

ErrorCode_t _flag_handle(Config_t* config, char* flag) {
    switch(flag[1]) {
        case 'p': {
            int port_num = atoi(flag + 2);
            if (port_num == 0) {
                // Make sure there is an error instead of just port number being
                // 0
                if (strlen(flag) != 3) {
                    // port number cannot be read
                    printf("Error in flag %s: can't read port number.\n", flag);
                    return Error;
                }
            }
            config->open_socket = TRUE;
            config->port = port_num;
            return Success;
        }
        case 'm': {
            config->log_memory = TRUE;
            return Success;
        }
        case 'h': {
            return Error;
        }
        default: {
            assert(FALSE);
            return Success;
        }
    }
}

void _interpreter_start(Config_t* config) {
    printf("Initializing VM\n");
    struct VM* vm = vm_make();
    
    if (config->log_memory == TRUE) {
        printf("Starting memory logger service\n");
        init_logger();
    }

    if (config->file_to_interpret != NULL) {
        printf("Interpreting file %s\n", config->file_to_interpret);
        struct Program* program = _interpret_file(vm, config->file_to_interpret);
        program_print(program);
        program_free(program);
    } else {
        if (config->open_socket == TRUE) {
            printf("Listening on port %d\n", config->port);
            Connection_t client = network_listen(config->port);
            _repl_start_remote(vm, client);
        } else {
            printf("Starting local session\n");
            _repl_start_local(vm);
        }
    }

    vm_free(vm);
}

void _repl_start_local(struct VM* vm) {
    // read in command
    char buffer[1024];
    Response_t* response = response_make_void();
    
    while (response_get_type(response) != ExitResponse) {
        printf("HASSLE> ");
        gets(buffer);
        response_free(response);
        response = _execute_command(vm, buffer);
        switch (response_get_type(response)) {
            case EvalStateResponse: {
                // TODO
                break;
            }
            case TermResponse: {
                // TODO
                break;
            }
            case VMDataResponse: {
                size_t size;
                uint8_t* data = response_get_data(response, &size);
                size_t i = 0;
                uint8_t counter = 0;
                while (i < size) {
                    if (counter == 16) {
                        counter = 0;
                        printf("\n");
                    }
                    uint8_t byte = data[i];
                    if (byte < 10) {
                        printf(" ");
                    }
                    if (byte < 100) {
                        printf(" ");
                    }
                    printf("%d ", data[i]);
                    i++;
                    counter++;
                }
                printf("\n");
                break;
            }
            case VoidResponse: {
                // TODO
                break;
            }
            case InvalidCommandResponse: {
                // TODO
                break;
            }
            case ExitResponse: {
                // TODO cleanup
                break;
            }
        }
    }

    response_free(response);
}

void _repl_start_remote(struct VM* vm, Connection_t conn) {
    printf("Starting remote repl\n");
    Response_t* response = response_make_void();
    int size;
    BOOL is_alive = TRUE;
    while ((response_get_type(response) != ExitResponse) && (is_alive == TRUE))
    {
        uint8_t* buffer = network_receive(conn, &size, &is_alive);
        if (buffer == NULL) {
            break;
        }
        printf("[RECEIVED] %s\n", (char*)buffer);
        response_free(response);
        response = _execute_command(vm, (char*)buffer);
        free_mem("_repl_start_remote", buffer);
        size_t resp_data_size;
        uint8_t* resp_data = response_get_data(response, &resp_data_size);
        network_send(conn, resp_data, (int)resp_data_size);
        // printf("Sent %d bytes\n", (int)resp_data_size);
    }
    response_free(response);
    network_close(conn);
}

struct Tree* test0() {
    return delta();
}

struct Tree* test1() {
    char* x = malloc(2);
    x[0] = 120; x[1] = 0;
    struct Tree* t_true = true();
    struct Tree* t_not = not();
    // struct Tree* t_x =
    //     tree_make_value(program_make(value_make_sym(x), NULL, NULL));
    // struct Tree* app = tree_make_apply(t_not, t_x);
    // return tree_make_apply(nStar("x", app), t_true);
    return tree_make_apply(t_not, t_true);
}

struct Tree* test2() {
    struct Tree* num1 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("12"))), NULL,
                NULL));
    struct Tree* num2 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("3"))), NULL,
                NULL));
    struct Tree* op =
        tree_make_value(
            program_make(value_make_primop(Add), NULL, NULL));
    return
        tree_make_apply(
            tree_make_apply(
                tree_make_apply(
                    delta(),
                    tree_make_apply(
                        tree_make_apply(
                            delta(),
                            num1),
                        num2)),
                delta()),
            op);
}

struct Tree* test2a() {
    return
        nBracket(str_cpy("x"),
            tree_make_value(program_make(value_make_sym("x"), NULL, NULL)));
}

struct Tree* test2b() {
    return
        nBracket(str_cpy("x"),
            tree_make_value(program_make(value_make_sym("y"), NULL, NULL)));
}

struct Tree* test2c() {
    struct Tree* apply_op =
        nBracket(str_cpy("x"),
            nBracket(str_cpy("y"),
                tree_make_value(program_make(value_make_sym("x"), NULL,
                        NULL))));
    struct Tree* num1 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("12"))), NULL,
                NULL));
    struct Tree* num2 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("3"))), NULL,
                NULL));
    return tree_make_apply(tree_make_apply(apply_op, num1), num2);
    // return apply_op;
}

struct Tree* test3a() {
    // \o. \x. \y. oxy
    struct Tree* apply_op =
        nBracket(str_cpy("o"), nBracket(str_cpy("x"), nBracket(str_cpy("y"),
            tree_make_apply(
                tree_make_apply(
                    tree_make_value(program_make(value_make_sym("o"), NULL,
                        NULL)),
                    tree_make_value(program_make(value_make_sym("x"), NULL,
                        NULL))),
                tree_make_value(program_make(value_make_sym("y"), NULL,
                    NULL))))));
    struct Tree* num1 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("12"))), NULL,
                NULL));
    struct Tree* num2 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("3"))), NULL,
                NULL));
    struct Tree* op =
        tree_make_value(
            program_make(value_make_primop(Add), NULL, NULL));
    // printf("Tree size: %llu\n", term_size(apply_op));
    return
        tree_make_apply(tree_make_apply(tree_make_apply(apply_op, op), num1),
            num2);
    // return term_apply(apply_op, num1);
}

struct Tree* test3b() {
    // \o. \x. \y. oxy
    struct Tree* apply_op =
        nStar(str_cpy("o"), nStar(str_cpy("x"), nStar(str_cpy("y"),
            tree_make_apply(
                tree_make_apply(
                    tree_make_value(program_make(value_make_sym("o"), NULL,
                        NULL)),
                    tree_make_value(program_make(value_make_sym("x"), NULL,
                        NULL))),
                tree_make_value(program_make(value_make_sym("y"), NULL,
                    NULL))))));
    struct Tree* num1 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("12"))), NULL,
                NULL));
    struct Tree* num2 =
        tree_make_value(
            program_make(
                value_make_rat(rational_from_string(str_cpy("3"))), NULL,
                NULL));
    struct Tree* op =
        tree_make_value(
            program_make(value_make_primop(Add), NULL, NULL));
    // printf("Tree size: %llu\n", term_size(apply_op));
    return
        tree_make_apply(tree_make_apply(tree_make_apply(apply_op, op), num1),
            num2);
    // return term_apply(apply_op, num1);
}

// struct Tree* test_va() {
//     // struct Tree* term1 = cV();
//     // printf("Tree size: %llu\n", term_size(term1)); // 909 in .v, 679 here
//     // struct Tree* term2 = cA();
//     // printf("Tree size: %llu\n", term_size(term2)); // 757 in .v, 563 here

//     struct Tree* n = term_make_rat(rational_from_string(str_cpy("12")));
//     // struct Tree* term = term_apply(term_apply(term_apply(cA(), cV()), cA()), n);

//     struct Tree* term00 = cA();
//     struct Tree* term01 = cV();
//     struct Tree* term0 = term_apply(term00, term01);
//     struct Tree* term1 = term_apply(term0, cA());
//     struct Tree* term = term_apply(term1, n);

//     return term;
// }

Response_t* _execute_command(struct VM* vm, char* cmd) {
    int token_len = str_get_token_end(cmd);
    char* cmds[] = {"file", "expr", "step", "run", "reset", "get", "exit"};
    int cmd_count = sizeof(cmds) / sizeof(cmds[0]);
    for (int i = 0; i < cmd_count; i++) {
        if (strlen(cmds[i]) == token_len &&
            strncmp(cmd, cmds[i], token_len) == 0)
        {
            switch(i) {
                case 0: {
                    // file
                    char* arg = str_get_substr(cmd, 1, TRUE);
                    if (arg != NULL) {
                        _interpret_file(vm, arg);
                    }
                    free_mem("execute_command/file", arg);
                    return response_make_void();
                }
                case 1: {
                    // expr
                    char* arg = str_get_substr(cmd, 1, TRUE);
                    if (arg != NULL) {
                        // vm_set_term(vm, parse_from_str(arg));
                        // TODO convert Expr_t* to struct Tree*
                        vm_populate(vm, test2c());
                    }
                    free_mem("execute_command/expr", arg);
                    return response_make_void();
                }
                case 2: {
                    // step
                    return response_make_eval_state(vm_step(vm));
                }
                case 3: {
                    // run
                    return response_make_program(vm_run(vm));
                }
                case 4: {
                    // reset
                    vm_reset(vm);
                    return response_make_void();
                }
                case 5: {
                    // get word_size
                    char* arg = str_get_substr(cmd, 1, FALSE);
                    uint8_t word_size;
                    if (arg == NULL) {
                        word_size = sizeof(size_t);
                    } else {
                        word_size = atoi(arg);
                    }
                    free_mem("execute_command/get", arg);
                    return response_make_vm_data(vm_serialize(vm, word_size));
                }
                case 6: {
                    // exit
                    return response_make_exit();
                }
            }
        }
    }
    return response_make_invalid_command();
}

void _print_help_message() {
    printf("HASSLE\n");
    printf("\n");
    printf("USAGE\n");
    printf("  hassle [-p[PORT_NUMBER]] [-m] [-h] [filename]\n");
    printf("\n");
    printf("DESCRIPTION\n");
    printf(
"  Hassle can interpret a file, or act as an interactive debugger. If a\n"
"  filename is given, it will evaluate the file's content. If the -p switch\n"
"  is specified, a listener port is opened, to which a debugger can connect.\n"
"  The filename and the -p flag can be specified together, but at least one\n"
"  of must be present.\n");
    printf("\n");
    printf("  -p[PORT_NUMBER]  Listens to a client on PORT_NUMBER\n");
    printf("  -m               Turns on memory logging (useful for debugging)\n");
    printf("  -h               Prints this message\n");
}

struct Program* _interpret_file(struct VM* vm, char* file_name) {
    // struct Tree* term = parse_from_file(file_name);
    struct Tree* tree = NULL; // TODO convert it from Expr_t*
    vm_populate(vm, tree);
    return vm_run(vm);
}

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("---------- Hassle ----------\n\n");
    setvbuf(stdout, (char *) NULL, _IONBF, 0); // make stdout line-buffered
    Config_t config = _config_init();

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            ErrorCode_t error_code = _flag_handle(&config, argv[i]);
            if (error_code == Error) {
                _print_help_message();
                return error_code;
            }
        } else {
            // Interpret the provided file
            config.file_to_interpret = argv[i];
        }
    }

    _interpreter_start(&config);
    
    return 0;
}

// int main(int argc, char *argv[]) {
//     printf("+--------+\n| Hassle |\n+--------+\n\n");

//     // \x. not x

//     char* x = malloc(2);
//     x[0] = 120; x[1] = 0;
//     struct Tree* not_x =
//         term_apply(nStar("x", term_apply(not(), term_make_sym(x))), true());
//     // struct Tree* not_x = term_apply(not(), term_make_sym("x"));

//     Serializer_t* ser = serializer_init(sizeof(size_t));
//     term_serialize(ser, not_x);
//     uint8_t* data = serializer_get_data(ser);
//     size_t data_size = serializer_get_data_size(ser);

//     for (size_t i = 0; i < data_size; i++) {
//         printf("%d, ", data[i]);
//     }
//     printf("\n");

//     struct Tree* result = eval(not_x);

//     ser = serializer_init(sizeof(size_t));
//     term_serialize(ser, result);
//     data = serializer_get_data(ser);
//     data_size = serializer_get_data_size(ser);

//     for (size_t i = 0; i < data_size; i++) {
//         printf("%d, ", data[i]);
//     }
    
//     return 0;
// }
