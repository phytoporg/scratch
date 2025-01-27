#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <util/util.h>

#include <net/socket.c>
#include <system/time.c>

#define CLIENT_TICK_FREQ 60
#define CLIENT_HEARTBEAT_FREQ 1

struct client_context
{
    int socket_handle;
    uint64_t last_heartbeat_ns;
};

bool _client_init(struct client_context* context, int port)
{
    if (!context)
    {
        return false;
    }

    context->socket_handle = -1;
    context->socket_handle = socket_create_udp();
    context->last_heartbeat_ns = 0;
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

void _client_heartbeat(struct client_context* context, int address, int port)
{
    uint8_t buffer[] = "alive";
    if (socket_send(context->socket_handle,
                    buffer,
                    sizeof(buffer),
                    address,
                    port) < 0)
    {
        fprintf(stderr, "Failed to say hello to server\n");
    }
}

bool _client_tick(struct client_context* context, int address, int port)
{
    const uint64_t now_ns = system_time_ns();
    const uint64_t diff_ns = now_ns - context->last_heartbeat_ns;
    const uint64_t heartbeat_threshold_ns = BILLION / CLIENT_HEARTBEAT_FREQ;
    if (diff_ns >= heartbeat_threshold_ns)
    {
        _client_heartbeat(context, address, port);
        context->last_heartbeat_ns = now_ns;
    }

    // Tick forever r/n
    return true;
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

    // 60hz client tick
    const uint64_t TICK_FREQ_NS = BILLION / CLIENT_TICK_FREQ;
    bool keep_ticking = true;
    do
    {
        const uint64_t start_ns = system_time_ns();
        keep_ticking = _client_tick(&context, server_address, server_port);
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
