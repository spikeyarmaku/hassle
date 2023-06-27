#include "network.h"

#include <stdio.h>

#include "config.h"
#include "memory.h"

struct MsgBuf {
    int capacity;
    int count;
    uint8_t* data;
};

typedef struct MsgBuf MsgBuf_t;

MsgBuf_t*   _network_create_message_buffer  ();
void        _network_append_message         (MsgBuf_t*, const uint8_t*, int);

MsgBuf_t* _network_create_message_buffer() {
    MsgBuf_t* buf = (MsgBuf_t*)allocate_mem(NULL, NULL, sizeof(MsgBuf_t));
    buf->capacity = 0;
    buf->count = 0;
    buf->data = NULL;
    return buf;
}

void _network_append_message(MsgBuf_t* buf, const uint8_t* data, int size) {
    if (buf->capacity < buf->count + size) {
        buf->capacity = buf->capacity == 0 ? 1024 : buf->capacity * 2;
        buf->data = (uint8_t*)allocate_mem(NULL, buf->data,
            sizeof(uint8_t) * buf->capacity);
    }

    for (int i = 0; i < size; i++) {
        buf->data[buf->count + i] = data[i];
    }
    buf->count += size;
}

Connection_t network_listen(uint16_t port) {
    #ifdef WIN32
    WSADATA wsadata;
    printf("Calling WSAStartup\n");
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed: %d\n", iResult);
        return -1;
    }
    #endif
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Error while creating socket: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Error while binding socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    if (listen(sock, 1) == SOCKET_ERROR) {
        printf("Error while listening: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // struct sockaddr client;
    // int client_size = sizeof(client);
    // SOCKET client_sock = accept(sock, &client, &client_size);
    SOCKET client_sock = accept(sock, NULL, NULL);
    if (client_sock == INVALID_SOCKET) {
        printf("Error while accepting connection: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    printf("Client connected\n");

    closesocket(sock);
    return client_sock;
}

void network_close(Connection_t sock) {
    closesocket(sock);
    WSACleanup();
}

void network_send(Connection_t sock, uint8_t* data, int length) {
    send(sock, (char*)data, (int)length, 0);
}

uint8_t* network_receive(Connection_t sock, int* size, BOOL* is_alive) {
    MsgBuf_t* msg_buf = _network_create_message_buffer();
    const int buf_temp_capacity = 1024;
    uint8_t buf_temp[1024];
    BOOL msg_complete = FALSE;

    while (msg_complete == FALSE) {
        // Read from socket
        printf("Reading from socket\n");
        int result = recv(sock, (char*)buf_temp, buf_temp_capacity, 0);

        // Check for errors
        if (result == SOCKET_ERROR) {
            int last_error = WSAGetLastError();
            printf("Error while reading from socket: %d\n", last_error);
            if (last_error == WSAEMSGSIZE) {
                // If there are more messages to be read
                printf("More messages are available\n");
                _network_append_message(msg_buf, buf_temp, buf_temp_capacity);
            } else {
                printf("All messages have been read\n");
                msg_complete = TRUE;
            }
        } else {
            if (result == 0) {
                // The connection is terminated
                *is_alive = FALSE;
                closesocket(sock);
            } else {
                _network_append_message(msg_buf, buf_temp, result);
            }
            printf("All messages have been read\n");
            msg_complete = TRUE;
        }
    }
    *size = msg_buf->count;
    msg_buf->data[msg_buf->count] = 0;
    return msg_buf->data;
}
