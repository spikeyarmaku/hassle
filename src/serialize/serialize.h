#ifndef _SERIALIZE_H_
#define _SERIALIZE_H_

#include <stdint.h>

typedef struct Serializer Serializer_t;

Serializer_t*   serializer_init             (uint8_t);
Serializer_t*   serializer_from_data        (uint8_t*);
void            serializer_write            (Serializer_t*, uint8_t);
void            serializer_write_string     (Serializer_t*, char*);
void            serializer_write_word       (Serializer_t*, size_t);
uint8_t         serializer_peek             (Serializer_t*);
char*           serializer_read_string      (Serializer_t*);
size_t          serializer_read_word        (Serializer_t*);
void            serializer_move_cursor      (Serializer_t*, size_t);
uint8_t         serializer_read             (Serializer_t*);
uint8_t         serializer_get_word_size    (Serializer_t*);
uint8_t*        serializer_get_data         (Serializer_t*);
size_t          serializer_get_data_size    (Serializer_t*);
void            serializer_free             (Serializer_t*);

#endif