#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __linux__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
#else
    _Static_assert (0, "Unsupported platform");
#endif

#define CREATE_ADDR(a, b, c, d) \
        ((a) << 24) | \
        ((b) << 16) | \
        ((c) <<  8) | \
        (d)

#define SERVER_PORT 30000
#define CLIENT_PORT 30001

#define COMMON_MTU 512

#endif // __COMMON_H__
