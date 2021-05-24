// Katsiaryna Yalovik 306460 
#ifndef RECEIVE_INCLUDED
#define RECEIVE_INCLUDED 1
#include <chrono>
#include <netinet/ip_icmp.h>
#include <cassert>
#include <tuple>
#include <stdexcept>
#include <arpa/inet.h>
#include <utility>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <ctime>
#include <netinet/ip.h>
class Receive{
    public:
    std::vector<std::tuple <const char*,int, std::chrono::time_point<std::chrono::high_resolution_clock>>> receivePackets(int sockfd, int current_ttl,int current_id);
    bool checkPacket( int id_seq, int id,int ttl, int current_ttl, int current_id);
    Receive(char* ip_addr);
    bool returnFlag();
    void checkAddr(char* addr1, char* addr2);
    private:
        int current_ttl;
        int current_seq;
        bool flag = false;
        char* addres;
        
};

#endif