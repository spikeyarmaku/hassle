#ifndef _RESPONSE_H_
#define _RESPONSE_H_

// #include "tree/vm.h"
#include "vm_simple/vm.h"
#include "tree/program.h"

enum ResponseType {EvalStateResponse, TermResponse, VMDataResponse,
                    VoidResponse, InvalidCommandResponse, ExitResponse};

typedef struct Response Response_t;

Response_t*         response_make_void              ();
Response_t*         response_make_vm_data           (struct VMData);
Response_t*         response_make_program           (struct Program*);
Response_t*         response_make_eval_state        (enum EvalState);
Response_t*         response_make_invalid_command   ();
Response_t*         response_make_exit              ();
enum ResponseType   response_get_type               (Response_t*);
uint8_t*            response_get_data               (Response_t*, size_t*);
void                response_free                   (Response_t*);

#endif