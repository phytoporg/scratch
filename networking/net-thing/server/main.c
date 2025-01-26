#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <net/socket.c>
#include <system/time.c>

#include <util/util.h>

struct client_connection 
{
    int client_id;
    int address;
    int port;
};

#define MAX_CONNECTIONS 2
struct server_context
{
    int socket_handle;
    int num_connections;
    struct client_connection connections[MAX_CONNECTIONS];
};

int _server_find_connection(struct server_context* context, int address, int port)
{
    for (int c = 0; c < MAX_CONNECTIONS; ++c)
    {
        struct client_connection* connection = &context->connections[c];
        if (connection->address == address && connection->port == port)
        {
            return c;
        }
    }

    return -1;
}

bool _server_init(struct server_context* context)
{
    if (!context)
    {
        return false;
    }

    context->socket_handle = -1;
    context->num_connections = 0;
    memset(context->connections, 0, sizeof(context->connections));
    for (int c = 0; c < MAX_CONNECTIONS; ++c)
    {
        context->connections[c].client_id = -1;
    }

    context->socket_handle = socket_create_udp();
    if (context->socket_handle <= 0)
    {
        fprintf(stderr, "Failed to create server socket\n");
        return false;
    }

    if (!socket_bind(context->socket_handle, SERVER_PORT))
    {
        fprintf(stderr, "Failed to bind server socket\n");
        return false;
    }

    if (!socket_set_nonblocking(context->socket_handle))
    {
        fprintf(stderr, "Failed to configure server socket as nonblocking\n");
        return false;
    }

    return true;
}

bool _server_tick(struct server_context* context)
{
    uint8_t buffer[COMMON_MTU] = {0};
    const size_t max_packet_size = sizeof(buffer);
    bool looping = true;
    while (looping)
    {
        int address, port;
        const int received = 
            socket_recv(context->socket_handle,
                        buffer,
                        max_packet_size,
                        &address,
                        &port);

        if (received > 0)
        {
            const int index = _server_find_connection(context, address, port);
            if (index < 0)
            {
                if (context->num_connections >= MAX_CONNECTIONS)
                {
                    fprintf(stderr, "Skipping new connection, already at max\n");
                    continue;
                }

                // Add a new connection
                struct client_connection* next_connection = 
                    &context->connections[context->num_connections];
                next_connection->client_id = context->num_connections;
                next_connection->address = address;
                next_connection->port = port;
                ++context->num_connections;

                fprintf(stdout,
                        "New connection: %d (port=%d)\n", 
                        next_connection->client_id,
                        next_connection->port);
            }
            else
            {
                struct client_connection* connection = 
                    &context->connections[index];
                fprintf(stdout,
                        "msg from existing client %d: %s\n",
                        connection->client_id,
                        buffer);
            }
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            looping = false;
        }
    }

    return true;
}

int main(int argc, char** argv)
{
    struct server_context context;
    if (!_server_init(&context))
    {
        return -1;
    }

    // 120hz server tick
    const uint64_t TICK_FREQ_NS = BILLION / 120;
    bool keep_ticking = true;
    do
    {
        const uint64_t start_ns = system_time_ns();
        keep_ticking = _server_tick(&context);
        const uint64_t end_ns = system_time_ns();

        const uint64_t diff = end_ns - start_ns;
        if (diff < TICK_FREQ_NS)
        {
            const uint64_t time_remaining_ns = (TICK_FREQ_NS - diff);
            sleep_ns(time_remaining_ns);
        }
    } while(keep_ticking);

    return 0;
}
