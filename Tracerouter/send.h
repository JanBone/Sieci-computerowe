// Katsiaryna Yalovik 306460 
#ifndef SEND_INCLUDED
#define SEND_INCLUDED 1
#include <stdio.h>
#include <stdexcept>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <regex>
#include <cassert>
#include <tuple>
#include <chrono>
#include <sys/time.h>
#include <ctime>


class Send{
    public:
    u_int16_t compute_icmp_checksum (const void *buff, int length);
    icmp MakeHeader(int id);
    void sendPacket(int sockfd,  char* ip_addr, int ttl, int id);

};

#endif