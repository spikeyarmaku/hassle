// TODO collect the strings to a string pool, and just use indexes instead of
// actual strings

#include "serialize.h"

// DEBUG
#include <stdio.h>

#include "config.h"
#include "memory.h"

struct Serializer {
    // How many bytes is a size_t
    // It is used to indicate the word size of the architecture the data was
    // serialized with. If the decoder architecture's word size is smaller,
    // it needs to account for that difference when deserializing the data
    uint8_t word_size;

    size_t data_capacity;
    size_t data_count; // How many bytes were written
    uint8_t* data;

    size_t cursor; // for reading data
};

void    _serializer_grow    (Serializer_t*);

void _serializer_grow(Serializer_t* serializer) {
    size_t new_capacity = serializer->data_capacity < BUFFER_SIZE ?
        BUFFER_SIZE : serializer->data_capacity * BUFFER_SIZE_MULTIPLY_FACTOR;
    serializer->data_capacity = new_capacity;
    
    serializer->data = (uint8_t*)allocate_mem("_serializer_grow",
        serializer->data, new_capacity * sizeof(uint8_t));
}

Serializer_t* serializer_init(uint8_t word_size) {
    Serializer_t* serializer = (Serializer_t*)allocate_mem("serializer_init",
        NULL, sizeof(Serializer_t));
    serializer->word_size = word_size;
    serializer->data = NULL;
    serializer->data_capacity = 0;
    serializer->data_count = 0;
    serializer->cursor = 0;

    serializer_write(serializer, word_size);
    return serializer;
}

Serializer_t* serializer_from_data(uint8_t* data) {
    Serializer_t* serializer = (Serializer_t*)allocate_mem("serializer_init",
        NULL, sizeof(Serializer_t));
    serializer->data = data;
    serializer->data_capacity = 0;
    serializer->data_count = 0;
    serializer->cursor = 0;
    
    serializer->word_size = serializer_read(serializer);
    return serializer;
}

void serializer_write(Serializer_t* serializer, uint8_t byte) {
    if (serializer->data_count == serializer->data_capacity) {
        _serializer_grow(serializer);
    }

    serializer->data[serializer->data_count] = byte;
    serializer->data_count++;
}

void serializer_write_string(Serializer_t* serializer, char* string) {
    if (string != NULL) {
        size_t counter = 0;
        while (*(string + counter) != 0) {
            serializer_write(serializer, *(string + counter));
            counter++;
        }
    }
    serializer_write(serializer, 0);
}

void serializer_write_word(Serializer_t* serializer, size_t word) {
    uint8_t buffer[sizeof(size_t)];
    word_to_bytes(word, buffer, serializer->word_size);

    for (uint8_t i = 0; i < serializer->word_size; i++) {
        serializer_write(serializer, buffer[i]);
    }
}

uint8_t serializer_peek(Serializer_t* serializer) {
    return serializer->data[serializer->cursor];
}

char* serializer_read_string(Serializer_t* serializer) {
    size_t string_length = 0;
    while (serializer->data[serializer->cursor + string_length] != 0) {
        string_length++;
    }
    char* string = (char*)allocate_mem("serializer_read_string", NULL,
        sizeof(char) * (string_length + 1));
    size_t counter = 0;
    while(counter <= string_length) {
        string[counter] = serializer_read(serializer);
        counter++;
    }
    return string;
}

size_t serializer_read_word(Serializer_t* serializer) {
    uint8_t bytes[sizeof(size_t)];
    for (uint8_t i = 0; i < serializer->word_size; i++) {
        bytes[i] = serializer_read(serializer);
    }
    return bytes_to_word(bytes, serializer->word_size);
}

void serializer_move_cursor(Serializer_t* serializer, size_t amount) {
    serializer->cursor += amount;
}

uint8_t serializer_read(Serializer_t* serializer) {
    uint8_t data = serializer_peek(serializer);
    serializer_move_cursor(serializer, 1);
    return data;
}

uint8_t serializer_get_word_size(Serializer_t* serializer) {
    return serializer->word_size;
}

uint8_t* serializer_get_data(Serializer_t* serializer) {
    return serializer->data;
}

size_t serializer_get_data_size(Serializer_t* serializer) {
    return serializer->data_count;
}

void serializer_free(Serializer_t* serializer) {
    free_mem("serializer_free/data", serializer->data);
    free_mem("serializer_free", serializer);
}

void serializer_free_toplevel(Serializer_t* serializer) {
    free_mem("serializer_free_toplevel", serializer);
}

void serializer_print(Serializer_t* serializer) {
    uint8_t counter = 0;
    for (size_t i = 0; i < serializer->data_count; i++) {
        uint8_t byte = serializer->data[i];
        if (byte < 10) {
            printf(" ");
        }
        if (byte < 100) {
            printf(" ");
        }
        printf("%d ", byte);
        counter++;
        if (counter == 16) {
            counter = 0;
            printf("\n");
        }
    }
}
