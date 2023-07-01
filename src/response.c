#include "response.h"

#include "vm\term.h"
#include "vm/vm.h"

// Return value of command execution

struct Response {
    size_t size;
    uint8_t* data;
};

// struct Response {
//     enum ResponseType type;
//     union {
//         enum EvalState eval_state;
//         Term_t* term;
//         struct VMData vm_data;
//     };
// };

Response_t* _allocate_response_data (size_t);

Response_t* _allocate_response_data(size_t byte_count) {
    Response_t* resp =
        (Response_t*)allocate_mem(NULL, NULL, sizeof(Response_t));
    resp->size = byte_count;
    resp->data =
        (uint8_t*)allocate_mem(NULL, NULL, sizeof(uint8_t) * byte_count);
    return resp;
}

enum ResponseType response_get_type(Response_t* resp) {
    return resp->data[0];
}

Response_t* response_make_void() {
    Response_t* response = _allocate_response_data(1);
    response->data[0] = (uint8_t)VoidResponse;
    return response;
}

Response_t* response_make_vm_data(struct VMData vm_data) {
    Response_t* response = _allocate_response_data(vm_data.data_size + 1);
    response->data[0] = (uint8_t)VMDataResponse;
    memcpy(response->data + 1, vm_data.data, vm_data.data_size);
    free_mem("response_make_vm_data", vm_data.data);
    return response;
}

Response_t* response_make_term(Term_t* term) {
    // Serializer will append a byte signifying word size before the serialized
    // term. Since this response doesn't need it, that byte can be used to
    // mark the response type instead, saving a memcpy call.

    Serializer_t* serializer = serializer_init(0); // 0 is fine for term
    term_serialize(serializer, term);
    size_t data_size = serializer_get_data_size(serializer);
    uint8_t* buf = serializer_get_data(serializer);

    Response_t* response = (Response_t*)allocate_mem(NULL, NULL,
        sizeof(Response_t));
    response->data = buf;
    response->data[0] = (uint8_t)TermResponse;
    response->size = data_size;

    // serializer_free_toplevel(serializer);

    return response;
}

Response_t* response_make_eval_state(enum EvalState eval_state) {
    Response_t* response = _allocate_response_data(2);
    response->data[0] = (uint8_t)EvalStateResponse;
    response->data[1] = (uint8_t)eval_state;
    return response;
}

Response_t* response_make_invalid_command() {
    Response_t* response = _allocate_response_data(1);
    response->data[0] = (uint8_t)InvalidCommandResponse;
    return response;
}

Response_t* response_make_exit() {
    Response_t* response = _allocate_response_data(1);
    response->data[0] = (uint8_t)ExitResponse;
    return response;
}

uint8_t* response_get_data(Response_t* resp, size_t* size) {
    *size = resp->size;
    return resp->data;
}

void response_free(Response_t* resp) {
    free_mem("response_free/data", resp->data);
    free_mem("response_free", resp);
}
