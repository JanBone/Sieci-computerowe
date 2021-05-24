// Katsiaryna Yalovik 306460 
#include "receive.h"

bool  Receive::checkPacket( int id_seq, int id,int ttl, int current_ttl, int current_id){
 if (id == getpid() && (id_seq <= current_id && id_seq >= current_id-3) && ttl == current_ttl){
      return true;
}

   return false;
}

Receive::Receive(char* ip_addr){
  addres = ip_addr;
}

bool Receive::returnFlag(){
  return flag;
}

void Receive::checkAddr(char* addr1, char* addr2){
   if (strcmp(addr1, addr2) == 0){
        flag = true;
  }
}


std::vector<std::tuple <const char*,int, std::chrono::time_point<std::chrono::high_resolution_clock>>> Receive::receivePackets(int sockfd, int current_ttl,int current_id){
  struct ip* ip_header;
  ssize_t		ip_header_len;
  ssize_t packet_len;
  fd_set descriptors;
  FD_ZERO(&descriptors);
  FD_SET(sockfd, &descriptors);
  struct sockaddr_in 	sender;	
	socklen_t sender_len = sizeof(sender);
	u_int8_t 	buffer[IP_MAXPACKET];
  char sender_ip_str[20];
  int ready, counter;
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  counter = 0;
  int id_seq, id, ttl;
  bool f;
  const char* p = " ";
  int size;

  std::vector<std::tuple <const char*,int, std::chrono::time_point<std::chrono::high_resolution_clock>>> storage;
  std::tuple<const char*, int, std::chrono::time_point<std::chrono::high_resolution_clock>> icmp_inf;
  while (true){
    ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv);
    if (ready < 0){
      throw std::invalid_argument( "Select failed");
    }
    if (ready == 0){
      break;
    }
      packet_len = recvfrom (sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
      if (packet_len <= 0){
        throw std::invalid_argument( "Packet receive failed");
      }
      inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
      checkAddr(sender_ip_str, addres);
      auto finish = std::chrono::high_resolution_clock::now();
      ip_header = (struct ip*)buffer;
      ip_header_len = ip_header->ip_hl * 4;
      struct icmphdr* h = (struct icmphdr*)(buffer + ip_header_len);
      struct icmp* original_icmp = (struct icmp*)(buffer + ip_header_len * 2 +8);
      
     if (h->type == ICMP_TIME_EXCEEDED) { 
       id_seq = original_icmp->icmp_seq;
       id = original_icmp->icmp_id;
       ttl = id_seq/3 + 1;
   
    }
    else if(h->type == ICMP_ECHOREPLY) {
       id_seq = h->un.echo.sequence;
       id = h->un.echo.id;

       ttl = id_seq/3 + 1;
    }
    
    if (checkPacket(id_seq,id,ttl,current_ttl,current_id)){
     f = false;
     size = storage.size();
        for(int i=0; i<size; i++){
          if (strcmp(sender_ip_str, std::get<0>(storage[i])) == 0){
              f = true;
          }
        }
        if (f){
          icmp_inf = std::make_tuple(p, id_seq, finish);
        }
        else{
            icmp_inf = std::make_tuple(sender_ip_str, id_seq, finish);
        }
        
        storage.push_back(icmp_inf);
   }
     counter++;
  }
  
      
     return storage;
  }









