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

#include "tree/term.h"
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
struct Term*    _interpret_file     (struct VM*, char*);
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
    struct VM* vm = vm_init();
    
    if (config->log_memory == TRUE) {
        printf("Starting memory logger service\n");
        init_logger();
    }

    if (config->file_to_interpret != NULL) {
        printf("Interpreting file %s\n", config->file_to_interpret);
        struct Term* term = _interpret_file(vm, config->file_to_interpret);
        term_print(term);
        // term_free(term);
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

struct Term* test1() {
    char* x = malloc(2);
    x[0] = 120; x[1] = 0;
    return term_apply(nStar("x", term_apply(not(), term_make_sym(x))), true());
}

struct Term* test2() {
    char* numstr1 = malloc(3);
    sprintf(numstr1, "12");
    char* numstr2 = malloc(3);
    sprintf(numstr2, "3");
    struct Term* num1 = term_make_rat(rational_from_string(numstr1));
    struct Term* num2 = term_make_rat(rational_from_string(numstr2));
    struct Term* op = term_make_primop(Add);
    return
        term_apply(
            term_apply(
                term_apply(
                    delta(),
                    term_apply(
                        term_apply(
                            delta(),
                            num1),
                        num2)),
                delta()),
            op);
}

struct Term* test3() {
    // \o. \x. \y. oxy
    // TODO add evals to x and y
    printf("test3()\n");
    char* varx = malloc(2);
    sprintf(varx, "x");
    char* varx2 = malloc(2);
    sprintf(varx2, "x");
    char* vary = malloc(2);
    sprintf(vary, "y");
    char* vary2 = malloc(2);
    sprintf(vary2, "y");
    char* varo = malloc(2);
    sprintf(varo, "o");
    char* varo2 = malloc(2);
    sprintf(varo2, "o");
    printf("Defining apply_op\n");
    struct Term* apply_op =
        nStar(varo2, nStar(varx2, nStar(vary2,
            term_apply(
                term_apply(
                    term_make_sym(varo), term_make_sym(varx)),
                term_make_sym(vary)))));
    printf("Defining rators / rands\n");
    char* numstr1 = malloc(3);
    sprintf(numstr1, "12");
    char* numstr2 = malloc(3);
    sprintf(numstr2, "3");
    struct Term* num1 = term_make_rat(rational_from_string(numstr1));
    struct Term* num2 = term_make_rat(rational_from_string(numstr2));
    struct Term* op = term_make_primop(Add);
    printf("Defining term\n");
    return term_apply(term_apply(term_apply(apply_op, op), num1), num2);
}

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
                        // TODO convert Expr_t* to struct Term*
                        vm_set_term(vm, test3());
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
                    return response_make_term(vm_run(vm));
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

struct Term* _interpret_file(struct VM* vm, char* file_name) {
    // struct Term* term = parse_from_file(file_name);
    struct Term* term = NULL; // TODO convert it from Expr_t*
    vm_set_term(vm, term);
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
//     struct Term* not_x =
//         term_apply(nStar("x", term_apply(not(), term_make_sym(x))), true());
//     // struct Term* not_x = term_apply(not(), term_make_sym("x"));

//     Serializer_t* ser = serializer_init(sizeof(size_t));
//     term_serialize(ser, not_x);
//     uint8_t* data = serializer_get_data(ser);
//     size_t data_size = serializer_get_data_size(ser);

//     for (size_t i = 0; i < data_size; i++) {
//         printf("%d, ", data[i]);
//     }
//     printf("\n");

//     struct Term* result = eval(not_x);

//     ser = serializer_init(sizeof(size_t));
//     term_serialize(ser, result);
//     data = serializer_get_data(ser);
//     data_size = serializer_get_data_size(ser);

//     for (size_t i = 0; i < data_size; i++) {
//         printf("%d, ", data[i]);
//     }
    
//     return 0;
// }
