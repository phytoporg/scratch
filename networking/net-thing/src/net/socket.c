#include <net/common.h>

int socket_create_udp()
{
    return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

bool socket_bind(int socket, int port)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(socket,
            (const struct sockaddr*)&address,
            sizeof(address)))
    {
        return false;
    }

    return true;
}

bool socket_set_nonblocking(int socket)
{
    const int non_blocking = 1;
    if (fcntl(socket,
             F_SETFL,
             O_NONBLOCK,
             non_blocking))
    {
        return false;
    }

    return true;
}

int socket_send(int socket, char* buffer, size_t len, int address, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_port = htons(port);

    const int sent = 
        sendto(socket,
               buffer,
               len,
               0,
               (struct sockaddr*)&addr,
               sizeof(addr));
    if (sent != len)
    {
        return -1;
    }

    return sent;
}

int socket_recv(int socket, char* buffer, size_t maxlen, int* address, int* port)
{
    *address = -1;
    *port = -1;

    struct sockaddr_in from;
    socklen_t len = sizeof(from);

    const int received = 
        recvfrom(socket,
                 buffer,
                 maxlen,
                 0,
                 (struct sockaddr*)&from,
                 &len);
    if (received > 0)
    {
        *address = ntohl(from.sin_addr.s_addr);
        *port = ntohs(from.sin_port);
    }

    return received;
}

void socket_close(int socket)
{
    close(socket);
}
