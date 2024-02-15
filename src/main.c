/*
TODO
- add Unicode support
- add locale support (decimal separators, etc.)
- use boehm's GC
- Check if the input expression is well-formed (all parens match)
- sprinkle `const` in appropriate places
*/

#include "config.h"
#include "global.h"

#include "parse/parse.h"
#include "serialize/serialize.h"

#include "network.h"
#include "memory.h"

// #include "tree/vm.h"
#include "vm_bytecode/vm.h"
#include "vm_bytecode/readback.h"

// DEBUG
// #include "tree/eval.h"
#include "tree/combinators.h"
#include "tree/tree_parse.h"


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
    struct VM* vm = vm_make(sizeof(size_t));
    
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

struct Tree* test_va() {
    // return tree_apply(tree_apply(cA(), cV()), cA());
    
    // K combinator in VA-calculus:
    // (A((V((V(VA))((V(VV))V)))(V((AV)A))))((AV)A)
    struct Tree* cK =
        tree_apply(
            tree_apply(cA(),
                tree_apply(
                    tree_apply(
                        cV(),
                        tree_apply(
                            tree_apply(cV(), tree_apply(cV(), cA())),
                            tree_apply(tree_apply(cV(), tree_apply(cV(), cV())),
                                cV()))),
                    tree_apply(cV(),
                        tree_apply(tree_apply(cA(), cV()), cA())))),
            tree_apply(tree_apply(cA(), cV()), cA()));
    return tree_apply(tree_apply(cK, nNat(3)), nNat(5));
}

struct Tree* test() {
    return test_va();
    // return tree_make_apply(tree_make_apply(and(), true()), false());
    // return tree_make_apply(cI(), _ref("x"));
    
    // return tree_make_apply(cI(), true());
    // return tree_make_apply(cI(), tree_make_apply(cI(), true()));
    // return cK();
    // return delta();
}

Response_t* _execute_command(struct VM* vm, char* cmd) {
    int token_len = str_get_token_end(cmd);
    char* cmds[] = {"file", "expr", "tree", "step", "run", "reset", "get",
        "exit"};
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
                        // vm_populate(vm, test());
                        // vm_from_tree(vm, test());
                        printf("Not implemented");
                        // vm_eval(vm);
                    }
                    free_mem("execute_command/expr", arg);
                    return response_make_void();
                }
                case 2: {
                    // tree
                    // TODO parse a tree expression
                    // every character that is not one of `[]{}()<>` is treated
                    // as a delta
                    // Otherwise, same result as calling "expr"
                    // vm_from_tree(parse_tree(...))
                    char* arg = str_get_substr(cmd, 1, TRUE);
                    vm_from_tree(vm, tree_from_string(arg));
                    return response_make_void();
                }
                case 3: {
                    // step
                    return response_make_eval_state(vm_step(vm));
                }
                case 4: {
                    // run
                    char* arg = str_get_substr(cmd, 1, FALSE);
                    if (arg == NULL) {
                        printf("Arg is null\n");
                        // return response_make_program(vm_run(vm));
                        vm_eval(vm);
                        return response_make_vm_data(vm_serialize(vm, 8));
                        // TODO use response_make_program instead
                        return NULL;
                    } else {
                        printf("Arg is not null\n");
                        // size_t step_count = atoi(arg);
                        // return
                        //     response_make_eval_state(
                        //         vm_run_steps(vm, step_count));
                        // TODO
                        return NULL;
                    }
                }
                case 5: {
                    // reset
                    vm_reset(vm);
                    return response_make_void();
                }
                case 6: {
                    // get word_size
                    // char* arg = str_get_substr(cmd, 1, FALSE);
                    // uint8_t word_size;
                    // if (arg == NULL) {
                    //     word_size = sizeof(size_t);
                    // } else {
                    //     word_size = atoi(arg);
                    // }
                    // free_mem("execute_command/get", arg);
                    // return response_make_vm_data(vm_serialize(vm, word_size));
                    // TODO use readback
                    
                    char* arg = str_get_substr(cmd, 1, FALSE);
                    uint8_t word_size;
                    if (arg == NULL) {
                        word_size = sizeof(size_t);
                    } else {
                        word_size = atoi(arg);
                    }
                    free_mem("execute_command/get", arg);
                    return response_make_vm_data(vm_serialize(vm, word_size));
                    // TODO use response_make_program instead
                }
                case 7: {
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
    vm_from_tree(vm, tree);
    // return vm_run(vm);
    // TODO
    return NULL;
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
