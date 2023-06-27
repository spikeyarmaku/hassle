/*
TODO
- add Unicode support
- add locale support (decimal separators, etc.)
- check memory allocations for leaks
- Check if the input expression is well-formed (all parens match)
*/

/*
Coding convenctions:

- each function is responsible for cleaning up the arguments they receive
  (callee-frees)
*/

#include "config.h"
#include "global.h"

#include "parse\parse.h"
#include "vm\vm.h"
#include "serialize\serialize.h"

#include "network.h"
#include "memory.h"
#include "response.h"

#include "vm/vm.h"

struct Config {
    BOOL open_socket;
    uint16_t port;
    BOOL log_memory;
    char* file_to_interpret;
};

typedef struct Config Config_t;

Config_t*   _config_init                    ();
ErrorCode_t _flag_handle                    (Config_t*, char*);
void        _interpreter_start              (Config_t*);
void        _print_help_message             ();
Term_t*     _interpret_file                 (VM_t*, char*);
void        _repl_start_local               (VM_t*);
void        _repl_start_remote              (VM_t*, Connection_t);
Response_t* _execute_command                (VM_t*, char*);

Config_t* _config_init() {
    Config_t* config = (Config_t*)allocate_mem(NULL, NULL, sizeof(Config_t));
    config->open_socket = FALSE;
    config->port = 0;
    config->log_memory = FALSE;
    config->file_to_interpret = NULL;
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
    VM_t* vm = vm_init();
    
    if (config->log_memory == TRUE) {
        printf("Starting memory logger service\n");
        init_logger();
    }

    if (config->file_to_interpret != NULL) {
        printf("Interpreting file %s\n", config->file_to_interpret);
        Term_t* term = _interpret_file(vm, config->file_to_interpret);
        term_print(term);
        term_free(term);
    } else {
        if (config->open_socket == TRUE) {
            printf("Listening on port %d\n", config->port);
            Connection_t client = network_listen(config->port);
            _repl_start_remote(vm, client);
        } else {
            _repl_start_local(vm);
        }
    }
}

void _repl_start_local(VM_t* vm) {
    // read in command
    char buffer[1024];
    Response_t* response = response_make_void();
    
    while (response_get_type(response) != ExitResponse) {
        gets(buffer);
        response = _execute_command(vm, buffer);
        switch (response_get_type(response)) {
            case EvalStateResponse: {
                break;
            }
            case TermResponse: {
                break;
            }
            case VMDataResponse: {
                break;
            }
            case VoidResponse: {
                break;
            }
            case InvalidCommandResponse: {
                break;
            }
            case ExitResponse: {
                break;
            }
        }
    }
}

void _repl_start_remote(VM_t* vm, Connection_t conn) {
    printf("Starting remote repl\n");
    Response_t* response = response_make_void();
    int size;
    BOOL is_alive = TRUE;
    while ((response_get_type(response) != ExitResponse) && (is_alive == TRUE))
    {
        uint8_t* buffer = network_receive(conn, &size, &is_alive);
        response = _execute_command(vm, (char*)buffer);
        size_t resp_data_size;
        uint8_t* resp_data = response_get_data(response, &resp_data_size);
        network_send(conn, resp_data, (int)resp_data_size);
        response_free(response);
    }
    network_close(conn);
}

Response_t* _execute_command(VM_t* vm, char* cmd) {
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
                    _interpret_file(vm, cmd + token_len + 1);
                    return response_make_void();
                }
                case 1: {
                    // expr
                    vm_set_control_to_expr(vm,
                        parse_from_str(cmd + token_len + 1));
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
                    int param_len = str_get_token_end(cmd + token_len + 1);
                    uint8_t word_size;
                    if (param_len == 0) {
                        word_size = sizeof(size_t);
                    } else {
                        word_size = atoi(cmd + token_len + 1);
                    }
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

Term_t* _interpret_file(VM_t* vm, char* file_name) {
    Expr_t* expr = parse_from_file(file_name);
    vm_set_control_to_expr(vm, expr);
    return vm_run(vm);
}

// If called with a file, run it, else start a REPL
int main(int argc, char *argv[]) {
    printf("---------- Hassle ----------\n\n");
    setvbuf(stdout, (char *) NULL, _IONBF, 0); /* make stdout line-buffered */
    Config_t* config = _config_init();

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            ErrorCode_t error_code = _flag_handle(config, argv[i]);
            if (error_code == Error) {
                _print_help_message();
                return error_code;
            }
        } else {
            // Interpret the provided file
            config->file_to_interpret = argv[i];
        }
    }

    _interpreter_start(config);
    
    return 0;
}

