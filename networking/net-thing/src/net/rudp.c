// Depends on socket.c, time.c

#include <string.h>

// I dunno I'm hungry
#define RUDP_PROTOCOL_ID 0xFEED
#define RUDP_PACKET_POOL_SIZE 16

enum rudp_status
{
    // A sentinel for uninitialized state
    RUDP_STATUS_INVALID = 0,

    // Connection is established
    RUDP_STATUS_CONNECTED,

    // Connection has timed out
    RUDP_STATUS_TIMEOUT,

    // Connection has gracefully disconnected
    RUDP_STATUS_DISCONNECTED
};

struct rudp_queued_packet
{
    uint8_t data[COMMON_MTU];
    size_t len;
};

typedef void(*rudp_read_fn)(int address, int port, uint8_t* data, size_t len, void* context);
typedef void(*rudp_status_fn)(enum rudp_status status, void* context);

struct rudp_conn
{
    int socket_handle;
    int remote_address;
    int remote_port;
    uint16_t ack;
    uint16_t remote_ack;
    uint64_t prev_recv_ns;
    rudp_read_fn read_callback;
    rudp_status_fn status_callback;
    void* context;
    struct rudp_queued_packet packet_pool[RUDP_PACKET_POOL_SIZE];
    int packets_to_send;
};

struct rudp_header
{
    uint16_t protocol_id;
    uint32_t ack_bits;
    bool     has_status;
};

bool
rudp_conn_init(
    int socket_handle,
    int remote_address,
    int remote_port,
    rudp_read_fn read_callback,
    rudp_status_fn status_callback,
    void* context,
    struct rudp_conn* connection_out)
{
    memset(connection_out, 0, sizeof(*connection_out));
    connection_out->socket_handle = socket_handle;
    connection_out->remote_address = remote_address;
    connection_out->remote_port = remote_port;
    connection_out->read_callback = read_callback;
    connection_out->status_callback = status_callback;
    connection_out->context = context;

    return true;
}

bool rudp_conn_close(struct rudp_conn* connection)
{
    // TODO: send goodbye
    // Caller closes the actual handle
    memset(connection, 0, sizeof(*connection));
    return true;
}

bool _rudp_tick_recv(struct rudp_conn* connection)
{
    uint8_t buffer[COMMON_MTU] = {0};
    const size_t max_packet_size = sizeof(buffer);
    int address, port;
    const int received =
        socket_recv(connection->socket_handle,
                    buffer,
                    max_packet_size,
                    &address,
                    &port);

    if (address != connection->remote_address ||
        port != connection->remote_port)
    {
        return false;
    }

    if (received >= sizeof(struct rudp_header))
    {
        struct rudp_header* header = (struct rudp_header*)(buffer);
        if (header->protocol_id != RUDP_PROTOCOL_ID)
        {
            fprintf(stderr, "Unexpected protocol ID, dropping packet\n");
            return false;
        }

        // TODO: Process ACK bits

        if (header->has_status)
        {
            // TODO: support for status payload
        }

        uint8_t* data = buffer + sizeof(*header);
        size_t len = received - sizeof(*header);
        connection->read_callback(address, port, data, len, connection->context);
    }

    return true;
}

bool _rudp_tick_send(struct rudp_conn* connection)
{
    uint8_t buffer[COMMON_MTU] = {0};
    struct rudp_header* header = (struct rudp_header*)(buffer);
    header->protocol_id = RUDP_PROTOCOL_ID;
    header->has_status = false; // TODO
    header->ack_bits = 0; // TODO

    const size_t header_size = sizeof(struct rudp_header);
    bool all_sends_succeeded = true;
    for (int p = 0; p < connection->packets_to_send; ++p)
    {
        struct rudp_queued_packet* packet = &connection->packet_pool[p];
        const size_t max_len = sizeof(buffer) - header_size;
        if (packet->len >= max_len)
        {
            fprintf(stderr, "Send packet too large - len: %d\n", packet->len);
            all_sends_succeeded = false;
            continue;
        }

        memcpy(buffer + header_size, packet->data, packet->len);

        size_t total_size = header_size + packet->len;
        const int sent =
            socket_send(
                connection->socket_handle,
                buffer,
                total_size,
                connection->remote_address,
                connection->remote_port);
        if (sent != total_size)
        {
            fprintf(stderr, "Send failed - total-size: %d sent: %d\n", total_size, sent);
            all_sends_succeeded = false;
        }
    }

    connection->packets_to_send = 0;
    return all_sends_succeeded;
}

bool rudp_tick(struct rudp_conn* connection)
{
    if (!_rudp_tick_recv(connection))
    {
        // TODO: Log something
    }

    if (!_rudp_tick_send(connection))
    {
        // TODO: Log something
    }

    return true;
}

bool rudp_send(struct rudp_conn* connection, void* data, size_t len)
{
    // TODO: enqueue packet data
    return false;
}