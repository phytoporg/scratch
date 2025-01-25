#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <net/socket.c>
#include <util/util.h>

struct client_context
{
    int socket_handle;
};

bool _client_init(struct client_context* context, int port)
{
    if (!context)
    {
        return false;
    }

    context->socket_handle = -1;
    context->socket_handle = socket_create_udp();
    if (context->socket_handle <= 0)
    {
        fprintf(stderr, "Failed to create client socket\n");
        return false;
    }

    if (!socket_bind(context->socket_handle, port))
    {
        fprintf(stderr, "Failed to bind client socket\n");
        return false;
    }

    if (!socket_set_nonblocking(context->socket_handle))
    {
        fprintf(stderr, "Failed to configure client socket as nonblocking\n");
        return false;
    }

    return true;
}

void _client_connect(struct client_context* context, int address, int port)
{
    uint8_t buffer[] = "hello";
    if (socket_send(context->socket_handle,
                    buffer,
                    sizeof(buffer),
                    address,
                    port) < 0)
    {
        fprintf(stderr, "Failed to say hello to server\n");
    }
}

int main(int argc, char** argv)
{
    int port = CLIENT_PORT;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    fprintf(stdout, "client using port %d\n", port);

    struct client_context context;
    if (!_client_init(&context, port))
    {
        return -1;
    }

    const int server_address = CREATE_ADDR(127, 0, 0, 1);
    const int server_port = SERVER_PORT;
    _client_connect(&context, server_address, server_port);

    return 0;
}
