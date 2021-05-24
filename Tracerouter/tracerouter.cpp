 // Katsiaryna Yalovik 306460 
 #include "receive.h"
 #include "send.h"
 #include "vector"
 #include <chrono> 
 #include <iostream>
 #include <chrono>
 #include <tuple>
using namespace std;


int line = 1;
std::vector<std::chrono::time_point<std::chrono::high_resolution_clock> >time_vec(91);

double calculate(int id1, int id2, int id3, std::chrono::time_point<std::chrono::high_resolution_clock> finish1, std::chrono::time_point<std::chrono::high_resolution_clock> finish2, std::chrono::time_point<std::chrono::high_resolution_clock> finish3){
      auto start1 = time_vec[id1];
      auto start2  = time_vec[id2];
      auto  start3 = time_vec[id3];
      return (std::chrono::duration_cast<std::chrono::milliseconds>(finish1 - start1).count() + std::chrono::duration_cast<std::chrono::milliseconds>(finish2 - start2).count() + std::chrono::duration_cast<std::chrono::milliseconds>(finish3 - start3).count())/3; 
}


void print(vector<tuple <const char*,int,  std::chrono::time_point< std::chrono::high_resolution_clock>>> vec){
    long long int t;
    int size;
    if (vec.size()==0){
      printf("%d. %s\n", line, "*");
      line++;
    }
    
    else {
      size = vec.size();
      if(size == 3){
          t = calculate(get<1>(vec[0]), get<1>(vec[1]), get<1>(vec[2]), get<2>(vec[0]),get<2>(vec[1]), get<2>(vec[2]));
          for (int k =0; k< size; k++){
              if (strcmp(std::get<0>(vec[k]), " ") != 0){
                printf("%d. %s ", line, get<0>(vec[k]));
                printf("%lld ms.\n", t);
                line++;
              }
          }
      }
        else{
          for (int k =0; k< size; k++){
              if (strcmp(std::get<0>(vec[k]), " ") != 0){
                printf("%d. %s ??\n", line, get<0>(vec[k]));
                line++;
              }
          }
        }
      }

    }
    
 int main(int argc, char* argv[])
{
  Receive receive(argv[1]);
  Send send;
  struct sockaddr_in addr;
  std::vector<std::tuple <const char*,int, std::chrono::time_point<std::chrono::high_resolution_clock>>> storage;

  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (argc != 2){
    throw std::invalid_argument( "Invalid ip addres format"); 
  }
  if ((inet_aton(argv[1], &addr.sin_addr) == 0)){
    throw std::invalid_argument( "Invalid ip addres"); 
    
  }
  if (sockfd < 0) {
    throw std::invalid_argument( "Socket creation failed"); 
    
  }
    
    int id = 0;
    for (int ttl = 1; ttl <= 30; ttl++){
     if (receive.returnFlag()){
        break;
      }

      for (int j = 1; j <=3; j++){
        auto start = std::chrono::high_resolution_clock::now();
        time_vec[id] = start;
        send.sendPacket(sockfd, argv[1], ttl, id);
        id++;
      }

      storage = receive.receivePackets(sockfd,ttl, id);
      print(storage);
      
    }
    
  
}