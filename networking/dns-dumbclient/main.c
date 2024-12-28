#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define DNS_ID 0x2020

//                                     1  1  1  1  1  1
//       0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                      ID                       |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                    QDCOUNT                    |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                    ANCOUNT                    |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                    NSCOUNT                    |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                    ARCOUNT                    |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

struct dns_header {
    uint16_t  id;
    uint16_t  flags;
    uint16_t  qdcount; // Number of entries in question section
    uint16_t  ancount; // Number of resource records in answer section
    uint16_t  nscount; // Number of name server resources in auth records section
    uint16_t  arcount; // Number of resource records in additional records section
};

//                                     1  1  1  1  1  1
//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                                               |
//    /                     QNAME                     /
//    /                                               /
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QTYPE                     |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QCLASS                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

struct dns_question {
    char* qname;
    uint16_t qtype;
    uint16_t qclass;
};

//                                    1  1  1  1  1  1
//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                                               |
//    /                                               /
//    /                      NAME                     /
//    |                                               |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                      TYPE                     |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     CLASS                     |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                      TTL                      |
//    |                                               |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                   RDLENGTH                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
//    /                     RDATA                     /
//    /                                               /
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ 

struct dns_answer {
    uint16_t compression;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    char* rdata;
};

char* dns_build_qname(char* hostname)
{
    const size_t hostnamelen = strlen(hostname);
    char* qname = (char*)calloc(hostnamelen + 2, sizeof(uint8_t));
    if (hostname == NULL || hostnamelen == 0)
    {
        return NULL;
    }

    // Reencode each segment as <seg-len><segment>
    char currentlen = 0;
    char* write = qname + 1;
    char* read = hostname;
    do
    {
        ++currentlen;
        if (*read != '.')
        {
            *write = *read;
        }
        else
        {
            *(write - currentlen) = currentlen;
            currentlen = 0;
        }

        ++read;
        ++write;
    } while (*read != '\0');

    return qname;
}

uint8_t* dns_build_request(char* hostname, size_t* packetlen)
{
    uint8_t* data = NULL;
    struct dns_header header = {
        .id = htons(DNS_ID),
        .flags = htons (0x0100), /* Q=0, RD=1 */
        .qdcount = 1,
        .ancount = 0,
        .nscount = 0,
        .arcount = 0
    };

    struct dns_question question = {
        .qtype = 1,  // A
        .qclass = 1, // IN
        .qname = dns_build_qname(hostname)
    };

    const size_t headlen = sizeof(header);
    const size_t qnamelen = strlen(hostname) + 2;
    const size_t questionlen = sizeof(uint16_t) * 2 + qnamelen;

    data = (uint8_t*)malloc(headlen + questionlen + qnamelen);
    memcpy(data, &header, headlen);
    memcpy(data + headlen, &question, questionlen);
    memcpy(data + headlen + questionlen, question.qname, qnamelen);

    *packetlen = headlen + questionlen + qnamelen;
    return data;
}

void dns_print_response(uint8_t* response, size_t length)
{
    struct dns_header* header = (struct dns_header*)response;
    if (!header->flags)
    {
        fprintf(stderr, "Invalid answer from host\n");
        return;
    }

    const uint16_t flags = htons(header->flags);
    const uint16_t ancount = htons(header->ancount);
    const uint16_t qdcount = htons(header->qdcount);
    const uint16_t nscount = htons(header->nscount);
    const uint16_t arcount = htons(header->arcount);
    fprintf(stdout, "Flags: 0x%04X\n", flags);
    fprintf(stdout, "ANCount: %d\n", ancount);
    fprintf(stdout, "QDCount: %d\n", qdcount);
    fprintf(stdout, "NSCount: %d\n", nscount);
    fprintf(stdout, "ARCount: %d\n", arcount);
    for (size_t i = 0; i < length; ++i)
    {
        const uint8_t byte = response[i];
        fprintf(stdout, "0x%02X ", byte);
    }
    fprintf(stdout, "\n");
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return -1;
    }

    char* hostname = argv[1];

    size_t packetlen = 0;
    uint8_t* packet = dns_build_request(hostname, &packetlen);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(0xd043dede);
    address.sin_port = htons(53);
    sendto(
        sockfd, packet, packetlen, 0,
        (struct sockaddr*)&address, (socklen_t)sizeof(address));

    socklen_t len = 0;
    uint8_t response[512];
    memset(response, 0, sizeof(response));
    ssize_t bytes = 
        recvfrom(sockfd, response, 512, 0, (struct sockaddr*)&address, &len);
    fprintf(stdout, "received: %d bytes\n", (int)bytes);

    dns_print_response(response, bytes);

    return 0;
}

