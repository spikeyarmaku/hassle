/*
TODO
- add a way to turn a tree into a term, using the closure of the tree (useful
for $vau) - name it `unpack`?

    - after $vau expansion: put "\x -> `unpack` body" in the control
    - `unpack` creates an update marker with the control's env, and gets
    replaced with "update_closure"
    - `update_closure` checks if the control is a variable - if it is, updates
    - the control with the value of that variable. If not, then updates the
    control's env with the topmost update marker, and removes the update marker
    from the stack

    ^ not good, the update markers from the variable lookup will go on top of
    the `update_closure` marker.
    However, we could add a `env_num with-env term` operator, that would change
    the current env of the control to `env_num`, and then put `term` in it


    new idea: (<op_name> signifies an operator)
    (<get-current-env> <set-env-to> body)
    // it becomes
    (<set-env-to> 54 body)
    // which becomes
    (body set-pair-env-to-54 set-leaf-env-to-54)
    // where set-pair-env-to-54 =
    // (\x -> \y -> (<set-env-to> 54 x) (<set-env-to> 54 y))
    // and set-leaf-env-to-54 = (\x -> <set-env-to> 54 x)
    // body evaluates...
    ((\p -> \l -> ((p x) y)) set-pair-env-to-54 set-leaf-env-to-54)
    ((set-pair-env-to-54 x) y)
    ((<set-env-to> 54 x) (<set-env-to> 54 y))


    update: (# is the mark to get the current frame index, #54 is a concrete
    frame index)
    at first, this is in the control:
    # (<SetEnv> body)
    next step:
    #54 (<SetEnv> body)
    then <SetEnv> notices that in the stack, the topmost element is not a frame
    index, therefore it knows it shall construct an extractor, so next step is:
    (body (\x. \y. (<SetEnv> 54 x) (<SetEnv> 54 y))) (\x. <SetEnv> 54 x)
    and now when <SetEnv> is eval'd, the topmost element on the stack will be a
    number

    ^ doesn't work. Either you have to store the frame in the term, in which
    case you would need the heap to serialize it, which you don't, or you need
    to store the frame index, in which case you need the heap when the primop is
    running, which you also don't have access to

    attempt #3: convert the tree into an actual expression. It will need to be
    marked as "not to evaluate" somewhere along the line
    - OR -
    find which closure's frame to overwrite

    attempt #4:
    define an <eval-with> primop, that takes two arguments, and constructs a
    closure from the first closure's term and the second closure's frame
    then, let <vau> construct:
    (x -> eval-with body x)


    decode =
        fix
            (\decode. \t.
                (t
                    (\x. \y.
                        (decode x)
                        (<make-app> (decode y)))
                    (\x. x)))
    x -> dummy ((decode body) <with-env>)
    <make-app> creates a closure with a NULL frame
    the VM treats everything with a NULL frame as a value
    <with-env> creates a closure from the term of closure1 and the frame of
    closure2


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

#include "eval/term.h"
#include "eval/eval.h"

// #include "response.h"
/*
struct Config {
    BOOL open_socket;
    uint16_t port;
    BOOL log_memory;
    char* file_to_interpret;
};

typedef struct Config Config_t;

Config_t    _config_init                    ();
ErrorCode_t _flag_handle                    (Config_t*, char*);
void        _interpreter_start              (Config_t*);
void        _print_help_message             ();
Term_t*     _interpret_file                 (VM_t*, char*);
void        _repl_start_local               (VM_t*);
void        _repl_start_remote              (VM_t*, Connection_t);
Response_t* _execute_command                (VM_t*, char*);

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
    VM_t* vm = vm_init();
    
    if (config->log_memory == TRUE) {
        printf("Starting memory logger service\n");
        init_logger();
    }

    if (config->file_to_interpret != NULL) {
        printf("Interpreting file %s\n", config->file_to_interpret);
        Term_t* term = _interpret_file(vm, config->file_to_interpret);
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

    // vm_free(vm);
}

void _repl_start_local(VM_t* vm) {
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

void _repl_start_remote(VM_t* vm, Connection_t conn) {
    printf("Starting remote repl\n");
    Response_t* response = response_make_void();
    int size;
    BOOL is_alive = TRUE;
    while ((response_get_type(response) != ExitResponse) && (is_alive == TRUE))
    {
        uint8_t* buffer = network_receive(conn, &size, &is_alive);
        // printf("[RECEIVED] %s\n", (char*)buffer);
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
                        vm_set_control_to_expr(vm, parse_from_str(arg));
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

Term_t* _interpret_file(VM_t* vm, char* file_name) {
    Expr_t* expr = parse_from_file(file_name);
    vm_set_control_to_expr(vm, expr);
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
*/

void test_simple() {
    // (λx. x x) (λx. x)
    // [Eval [Apply (λx. [Apply x x]) (λx. x)]]
    struct Term* term =
        term_make_app(
            term_make_abs("x",
                term_make_app(
                    term_make_var("x"),
                    term_make_var("x"))),
            term_make_abs("x",
                term_make_var("x")));

    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be (\x. x)
}

void test_lambda() {
    // (λlambda. (lambda x x)) ([Lam])
    // [Apply [Lambda lambda [Apply [Apply lambda x] x]] [Lam])
    struct Term* term1 =
        term_make_abs("lambda",
            term_make_app(
                term_make_app(
                    term_make_var("lambda"), term_make_var("x")),
                term_make_var("x")));
    struct Term* term = term_make_app(term1, term_make_lam());

    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be (\x. x)
}

void test_lambda_2() {
    // (\lambda. (lambda "x" x)) (\n. \b. [Lam [Sym n] b])
    struct Term* term1 =
        term_make_abs("lambda",
            term_make_app(
                term_make_app(
                    term_make_var("lambda"),
                    term_make_lit(literal_make_string("x"))),
                term_make_var("x")));
    struct Term* lambda =
        term_make_abs("n",
            term_make_abs("b",
                term_make_app(
                    term_make_app(
                        term_make_lam(),
                        term_make_app(
                            term_make_sym(),
                            term_make_var("n"))),
                    term_make_var("b"))));
    struct Term* term = term_make_app(term1, lambda);

    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be (\x. x)
}

void test3() {
    // (\lambda. ((lambda x x) y)) Lam
    // [APPLY [LAMBDA lambda [APPLY [APPLY [APPLY lambda x] x] y]] [Lam]]
    struct Term* term =
        term_make_app(
            term_make_abs("lambda",
                term_make_app(
                    term_make_app(
                        term_make_app(
                            term_make_var("lambda"),
                            term_make_var("x")),
                        term_make_var("x")),
                    term_make_var("y"))),
            term_make_lam());
    
    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be y
}

// Test fst
void test4() {
    // \x. \y. x
    struct Term* fst =
        term_make_abs("x", term_make_abs("y", term_make_var("x")));
    struct Term* term =
        term_make_app(
            term_make_app(fst, term_make_var("x")),
            term_make_var("y"));

    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be: x
}

// \f. (\x. (f x x)) (\x. (f x x))
// struct Term* term_make_fn_fix() {
//     struct Term* common =
//         term_make_abs("x",
//             term_make_app(
//                 term_make_app(
//                     term_make_var("f"),
//                     term_make_var("x")),
//                 term_make_var("x")));
//     return term_make_abs("f", term_make_app(common, common));
// }

// // Strict fixpoint combinator:
// // \f. (\x. (f (\v. x x v))) (\x. (f (\v. x x v)))
struct Term* term_make_fn_fix() {
    struct Term* common =
        term_make_abs("fix_x",
            term_make_app(
                term_make_var("fix_f"),
                term_make_abs("fix_v",
                    term_make_app(
                        term_make_app(
                            term_make_var("fix_x"),
                            term_make_var("fix_x")),
                        term_make_var("fix_v")))));
    return term_make_abs("fix_f", term_make_app(common, common));
}

// // \x. x
struct Term* term_make_fn_id() {
    return term_make_abs("id_x", term_make_var("id_x"));
}

// eval-pair   = x -> y -> (eval x) y
// eval        = t -> t id eval-pair
// fix (\eval. \t. (t (\x. (Sym x))) (\x. \y. (eval x) y))
// TODO deal with trees of strings instead of trees of terms
// also TODO change symbols to strings in test7()
struct Term* term_make_fn_eval() {
    return
        term_make_app(
            term_make_var("fix"),
            term_make_abs("eval",
                term_make_abs("eval_t",
                    term_make_app(
                        term_make_app(
                            term_make_var("eval_t"),
                            term_make_abs("eval_x",
                                term_make_app(
                                    term_make_sym(),
                                    term_make_var("eval_x")))),
                        term_make_abs("eval_x",
                            term_make_abs("eval_y",
                                term_make_app(
                                    term_make_app(
                                        term_make_var("eval"),
                                        term_make_var("eval_x")),
                                    term_make_var("eval_y"))))))));
}

// // \x. \y. \l. \p. p x y
struct Term* term_make_fn_pair() {
    return
        term_make_abs("pair_x",
            term_make_abs("pair_y",
                term_make_abs("pair_l",
                    term_make_abs("pair_p",
                        term_make_app(
                            term_make_app(
                                term_make_var("pair_p"),
                                term_make_var("pair_x")),
                            term_make_var("pair_y"))))));
}

// // \x. \l. \p. l x
struct Term* term_make_fn_leaf() {
    return
        term_make_abs("leaf_x",
            term_make_abs("leaf_l",
                term_make_abs("leaf_p",
                    term_make_app(
                        term_make_var("leaf_l"), term_make_var("leaf_x")))));
}

struct Term* term_wrap(char* symbol, struct Term* meaning, struct Term* term) {
    return term_make_app(term_make_abs(symbol, term), meaning);
}

struct Term* term_make_fn_lambda() {
    // lambda = \n. \b. [Lam [Sym (eval n)] b]
    return
        term_make_abs("lambda_name",
            term_make_abs("lambda_body",
                term_make_app(
                    term_make_app(
                        term_make_lam(),
                        term_make_app(
                            term_make_sym(),
                            term_make_app(
                                term_make_var("eval"),
                                term_make_var("lambda_name")))),
                    term_make_var("lambda_body"))));
}

void test_eval() {
    // (eval (leaf "x"))
    struct Term* sub_term =
        term_make_app(
            term_make_var("eval"),
            term_make_app(
                term_make_var("leaf"),
                term_make_lit(literal_make_string("id"))));
    struct Term* term =
        term_wrap("id", term_make_fn_id(),
        term_wrap("fix", term_make_fn_fix(),
        term_wrap("eval", term_make_fn_eval(),
        term_wrap("leaf", term_make_fn_leaf(),
            sub_term))));
    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be: x
}

void test_eval_2() {
    // (eval ((pair (leaf "id")) (leaf "x")))
    struct Term* sub_term =
        term_make_app(
            term_make_var("eval"),
            term_make_app(
                term_make_app(
                    term_make_var("pair"),
                    term_make_app(
                        term_make_var("leaf"),
                        term_make_lit(literal_make_string("id")))),
                term_make_app(
                    term_make_var("leaf"),
                    term_make_lit(literal_make_string("x")))));
    struct Term* term =
        term_wrap("id", term_make_fn_id(),
        term_wrap("fix", term_make_fn_fix(),
        term_wrap("eval", term_make_fn_eval(),
        term_wrap("pair", term_make_fn_pair(),
        term_wrap("leaf", term_make_fn_leaf(),
            sub_term)))));
    term_print(term); printf("\n");
    term_print(eval(term));
}

// https://www.lesswrong.com/posts/D4PYwNtYNwsgoixGa/intro-to-hacking-with-the-lambda-calculus
// TODO test this: ((\y. (\x. (x y))) x)
void test6() {
    struct Term* term =
        term_make_app(
            term_make_abs("y",
                term_make_abs("x",
                    term_make_app(
                        term_make_var("x"),
                        term_make_var("y")))),
            term_make_var("x"));
    
    term_print(term); printf("\n");
    term_print(eval(term));

    // Output should be: (x0 x)
}

// test `(($lambda x (eval x)) y)`
// (pair (pair (pair (leaf $vau) (leaf x)) (pair (leaf eval) (leaf x))) (leaf y))
// ((pair ((pair ((pair (leaf $vau)) (leaf x))) ((pair (leaf eval)) (leaf x)))) (leaf y))
void test7() {
    struct Term* term =
        term_make_app(
            term_make_app(
                term_make_var("pair"),
                term_make_app(
                    term_make_app(
                        term_make_var("pair"),
                        term_make_app(
                            term_make_app(
                                term_make_var("pair"),
                                term_make_app(
                                    term_make_var("leaf"),
                                    term_make_var("$lambda"))),
                            term_make_app(
                                term_make_var("leaf"),
                                term_make_var("x")))),
                    term_make_app(
                        term_make_app(
                            term_make_var("pair"),
                            term_make_app(
                                term_make_var("leaf"),
                                term_make_var("eval"))),
                        term_make_app(
                            term_make_var("leaf"),
                            term_make_var("x"))))),
            term_make_app(
                term_make_var("leaf"),
                term_make_var("y")));

    struct Term* eval_term = term_make_app(term_make_var("eval"), term);

    struct Term* final_term =
        term_wrap("fix", term_make_fn_fix(),
        term_wrap("id", term_make_fn_id(),
        term_wrap("eval", term_make_fn_eval(),
        term_wrap("leaf", term_make_fn_leaf(),
        term_wrap("pair", term_make_fn_pair(),
        term_wrap("$lambda", term_make_fn_lambda(),
            eval_term))))));
    term_print(final_term); printf("\n");
    
    struct Term* evaled = eval(final_term);
    // term_print_pretty(evaled); printf("\n");
    term_print(evaled);

    // Output should be:
    // [APPLY [APPLY [LAMBDA f [APPLY [LAMBDA x [APPLY [APPLY f x] x]] [LAMBDA x [APPLY [APPLY f x] x]]]] [LAMBDA eval [LAMBDA t [APPLY [APPLY t [LAMBDA x [LAMBDA y [APPLY [APPLY eval x] y]]]] [LAMBDA x x]]]]] [APPLY [APPLY [LAMBDA x [LAMBDA y [LAMBDA l [LAMBDA p [APPLY [APPLY p x] y]]]]] [APPLY [APPLY [LAMBDA x [LAMBDA y [LAMBDA l [LAMBDA p [APPLY [APPLY p x] y]]]]] [APPLY [APPLY [LAMBDA x [LAMBDA y [LAMBDA l [LAMBDA p [APPLY [APPLY p x] y]]]]] [APPLY [LAMBDA x [LAMBDA l [LAMBDA p [APPLY l x]]]] [LAM1]]] [APPLY [LAMBDA x [LAMBDA l [LAMBDA p [APPLY l x]]]] x]]] [APPLY [APPLY [LAMBDA x [LAMBDA y [LAMBDA l [LAMBDA p [APPLY [APPLY p x] y]]]]] [APPLY [LAMBDA x [LAMBDA l [LAMBDA p [APPLY l x]]]] [APPLY [LAMBDA f [APPLY [LAMBDA x [APPLY [APPLY f x] x]] [LAMBDA x [APPLY [APPLY f x] x]]]] [LAMBDA eval [LAMBDA t [APPLY [APPLY t [LAMBDA x [LAMBDA y [APPLY [APPLY eval x] y]]]] [LAMBDA x x]]]]]]] [APPLY [LAMBDA x [LAMBDA l [LAMBDA p [APPLY l x]]]] x]]]] [APPLY [LAMBDA x [LAMBDA l [LAMBDA p [APPLY l x]]]] y]]]
    // y
}

// TODO implement eval, and see if the encoded version of the input program
// evaluates correctly
// Example: `(($vau x (eval x)) 1)`
// the actual program should look like something like:
// [APPLY eval <pair (pair (pair (leaf $vau) ...))>]
// where <...> is the translated version of ..., i was just lazy to write it out
// (($vau x (eval x)) y)
// (pair (pair (pair (leaf $vau) (leaf x)) (pair (leaf eval) (leaf x))) (leaf y))
int main(int argc, char *argv[]) {
    printf("+--------+\n| Hassle |\n+--------+\n\n");

    test_eval();
    
    return 0;
}
