//Katsiaryna Yalovik
#include <chrono>
#include <iostream>
#include <ctime>
#include <netinet/ip.h>
#include <math.h>
#include <netinet/ip_icmp.h>
#include <cassert>
#include <fstream>
#include <tuple>
#include <stdexcept>
#include <arpa/inet.h>
#include <utility>
#include <cstring>
#include <stdio.h>
#include <string>
#include <vector>
#include <unistd.h>
using namespace std;
int port = 54321;
struct packet{
	bool to_delete = false;
	string network_addres;
	string source;
	string distance;
	uint32_t dist;
	int mask;
	std::chrono::time_point< std::chrono::high_resolution_clock> time_to_delete;
	

};
uint32_t inf = pow(2,32)-1;
struct network_attr{
	string network_addr;
	string broadcast;
	int mask;
	uint32_t dist;
	string router_ip;
	string my_ip;
	std::chrono::time_point< std::chrono::high_resolution_clock> time;
};
struct upd_packet {
		uint32_t dist;
		uint8_t mask ;
		struct in_addr addr;
  	};
vector<packet> route_table;
vector<network_attr> naighbours;


void send_packets(int sockfd){
	struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	uint32_t dist, neDist, neAddr, numBytesToSend;
	uint8_t mask ;

	struct in_addr addr;
	int n_size = naighbours.size();
	int r_size = route_table.size();
	for (int j=0; j<n_size; j++){
		if (inet_pton(AF_INET, naighbours[j].broadcast.c_str(), &server_address.sin_addr) == 0) {
			fprintf(stderr, "Failed to convert: %s\n", strerror(errno));
		}
		for (int i = 0; i < r_size; i++){
			if (inet_aton(route_table[i].network_addres.c_str(), &addr) == 0) {  
				fprintf(stderr, "Failed to convert: %s\n", strerror(errno));
    		}
			uint8_t buf[64];
			uint8_t * b = buf;
			mask = route_table[i].mask;
			dist = route_table[i].dist;

			neDist = htonl(dist);
			neAddr = htonl(addr.s_addr);
			memcpy(b, &neAddr, sizeof(neAddr));
			b += sizeof(neAddr);
			memcpy(b, &mask, sizeof(mask));
			b += sizeof(mask);
			memcpy(b, &neDist, sizeof(neDist));
			b += sizeof(neDist);

			numBytesToSend = (b-buf);
			
			if (sendto(sockfd, buf, numBytesToSend, 0,(struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
				fprintf(stderr, "Send error: %s\n", strerror(errno));
			};
			
		}
	}

}

void check_machine(){
	auto finish = std::chrono::high_resolution_clock::now();
	double time;
	string network_address, network_ip;
	vector<int> to_delete;
	int n_size = naighbours.size();
	int r_size = route_table.size();
	for (int i =0; i < n_size; i++){
		time = std::chrono::duration_cast<std::chrono::seconds>(finish - naighbours[i].time).count();
		if(time > 60){
			network_ip = naighbours[i].router_ip;
			for(int j=0; j < r_size; j++){
				if (network_ip.compare(route_table[j].source)== 0){
					if (route_table[j].dist != inf){
						route_table[j].dist = inf;
						route_table[j].time_to_delete =std::chrono::high_resolution_clock::now();
						route_table[j].to_delete = true;
					}
					
				}
			}
		}
		
	}
	
}

in_addr create_submask(int len){
	int l=len;
	int counter = 0;
	int num;
	std::string result;
	while(true){
		if (l-8 >= 0){
			result += "255";
			l=l-8;
		}
		else{
			if (l > 0){
				num = (int)(255 - pow(2, 8-l)+1);
				result+=to_string(num);
				l = 0;

			}
			else{
				result+="0";
			}
		}
		counter++;
		if (counter < 4){
			result+=".";
		}
		else{
			break;
		}
	}
	struct in_addr subnetmask;
	const char * c = result.c_str();
	if (inet_pton(AF_INET,c, &subnetmask.s_addr)< 0){
		fprintf(stderr, "Cannot conver address: %s\n", strerror(errno));
	}   

	return subnetmask;
}
void printRouteTable(){
	bool visited;
	int n_size = naighbours.size();
	int r_size = route_table.size();
	cout << "______________________________________________\n";
	for(int i =0; i < r_size; i++){
		visited = false;
		for(int j =0; j < n_size; j++){
			if (route_table[i].network_addres.compare(naighbours[j].network_addr) == 0){
				cout << route_table[i].network_addres << " distance " << naighbours[j].dist << " connected directly\n";
				visited = true;
				break;
			}
		}
		if(!visited){
			cout << route_table[i].network_addres << " distance " << route_table[i].dist << " via "<< route_table[i].source <<"\n";
		}
	}
	cout << "______________________________________________\n";
}

string network_addr(int mask, string ipaddress){
	char ip_str[20];
	in_addr subnetmask= create_submask(mask);
    struct in_addr ip_addr;
	const char * c = ipaddress.c_str();
	inet_pton(AF_INET, c, &ip_addr.s_addr);
	unsigned long final_ip = ip_addr.s_addr & subnetmask.s_addr;
    inet_ntop(AF_INET, &final_ip, ip_str, sizeof(ip_str));
	string wynik = (string) ip_str;
	return wynik;
}
string broadcast(int mask, string ipaddress){
	char ip_str[20];
	in_addr subnetmask= create_submask(mask);
    struct in_addr ip_addr;
	const char * c = ipaddress.c_str();
	inet_pton(AF_INET, c, &ip_addr.s_addr);
	unsigned long final_ip = ip_addr.s_addr | ~(subnetmask.s_addr);
    inet_ntop(AF_INET, &final_ip, ip_str, sizeof(ip_str));
	string wynik = (string) ip_str;
	return wynik;
}
bool checkmy(string ip){
	int n_size = naighbours.size();
	for(int i =0; i < n_size; i++){
		if(ip.compare(naighbours[i].my_ip)==0){ 			  														
				
			return true;
		}	
	}
	return false;
}

bool mynetwork(string ip){
	int n_size = naighbours.size();
	for(int i =0; i < n_size; i++){
		if(ip.compare(naighbours[i].network_addr)==0){
			return true;
		}
	}
	return false;
}
void check_naibourghs(string source){
	string net_work_addr;
	auto start = std::chrono::high_resolution_clock::now();
	int n_size = naighbours.size();
	for(int i =0; i < n_size; i++){
		net_work_addr = network_addr(naighbours[i].mask, source);
		if(net_work_addr.compare(naighbours[i].network_addr)==0){ 			  														
				naighbours[i].time = start;
				naighbours[i].router_ip = source;

		}																	 
	}



}
packet make_triplet(string ip_sub, int mask, int dist, string source){
	packet p;
	p.mask = mask;
	p.dist = dist;
	p.source = source;
	string network_addr_ip = network_addr(mask, ip_sub);
	p.network_addres = network_addr_ip;
	return p;
}

int find_dist(string ip_addr){
	int n_size = naighbours.size();
	for (int i = 0; i < n_size; i++){
		string networkaddr = network_addr(naighbours[i].mask, ip_addr);
		if (networkaddr.compare(naighbours[i].network_addr) == 0){
			return naighbours[i].dist;
		}
	}
	return inf;   

}
void delete_expired(){
	double time;
	vector<int> to_delete;
	int r_size = route_table.size();
	for(int i=0; i<r_size; i++){
		if(route_table[i].to_delete){
			auto finish = std::chrono::high_resolution_clock::now();
			time = std::chrono::duration_cast<std::chrono::seconds>(finish - route_table[i].time_to_delete).count();

			if (time > 30){
				if(!mynetwork(route_table[i].network_addres)){
					to_delete.push_back(i);
				}
				
			}
		}
	}
	int size = to_delete.size();
	int i =0;
	while(i < size){
		route_table.erase(route_table.begin() + to_delete[i]-i);
		i++;
	}


}
void update_table(packet datagram){
	bool visited = false;
	int distance;
	uint64_t result;
	int r_size = route_table.size();
		for (int i = 0; i < r_size; i++){
			if ((route_table[i].network_addres).compare(datagram.network_addres) == 0){
					if(datagram.dist == inf){
						if(route_table[i].dist != inf){
							route_table[i].dist = inf;
							route_table[i].to_delete = true;
							auto start = std::chrono::high_resolution_clock::now();
							route_table[i].time_to_delete = start;
						}
					}
					else{
						distance = find_dist(datagram.source);
						result =  uint64_t (datagram.dist) + uint64_t(distance);
						if (uint64_t(route_table[i].dist) > result){
							route_table[i].dist = result;
							route_table[i].source =  datagram.source;
						}
						
						route_table[i].to_delete = false;
					}

				
				visited = true;

			}
		}

	if (!visited){
		if(datagram.dist != inf){
			datagram.dist = datagram.dist + find_dist(datagram.source); 
			route_table.push_back(datagram); 
		}
		
	}

	delete_expired();
}



void receive(int sockfd){

	struct timeval tv;
  	tv.tv_sec = 5;
  	tv.tv_usec = 0;
  	
	struct sockaddr_in 	sender;
	socklen_t 			sender_len = sizeof(sender);

	 struct msg {
		uint8_t num;
		uint32_t dist;
		struct in_addr addr;
	};
    int ready;
	double time;
	
	struct sockaddr_in server_address;


		bzero (&server_address, sizeof(server_address));
		server_address.sin_family      = AF_INET;
		server_address.sin_port        = htons(port);
		server_address.sin_addr.s_addr = htons(INADDR_ANY);
		if (bind (sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
		 {
			fprintf(stderr, "bind error: %s\n", strerror(errno));
			
		}

	auto start = std::chrono::high_resolution_clock::now();
	for (;;) {
		fd_set descriptors;
  		FD_ZERO(&descriptors);
  		FD_SET(sockfd, &descriptors);
		ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv);

		if (ready > 0){
			uint8_t buf[64];
			ssize_t numBytesReceived = recvfrom (sockfd, buf, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
			if (numBytesReceived < 0){
				fprintf(stderr, "Receive packet failed: %s\n", strerror(errno));
			}
			uint8_t * b = buf;
    		uint8_t * fib = buf+numBytesReceived;
			msg m;

			   if ((uint32_t)(fib-b) >= sizeof(m.addr.s_addr))
          		{
			  
             		uint32_t neAddr;
             		memcpy(&neAddr, b, sizeof(neAddr));
             		m.addr.s_addr = ntohl(neAddr);
					b += sizeof(neAddr);

					 if ((uint32_t)(fib-b) >= sizeof(m.num))
       				{

		  				memcpy(&m.num, b, sizeof(m.num));
          				b += sizeof(m.num);


						if ((uint32_t)(fib-b) >= sizeof(m.dist))
    					{	
       						uint32_t neDist;
       						memcpy(&neDist, b, sizeof(neDist));
       						b += sizeof(neDist);
       						m.dist = ntohl(neDist);  
   						}
					}
          		}

			char ip_from_dataram[20];
			inet_ntop(AF_INET, &(m.addr.s_addr), ip_from_dataram, sizeof(ip_from_dataram));
			char sender_ip_str[20];
			inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
			if (!checkmy(sender_ip_str)){
				check_naibourghs(sender_ip_str);
				update_table(make_triplet(ip_from_dataram, m.num, m.dist,sender_ip_str));
			}
			

		}
		check_machine();
		delete_expired();

	
		auto finish = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration_cast<std::chrono::seconds>(finish - start).count() ;
		if (time >= 20){
			printRouteTable();
			send_packets(sockfd);
			start = std::chrono::high_resolution_clock::now();
		}
		tv.tv_sec = 5;
  		tv.tv_usec = 0;

	}




}

void prepare(string line){
	string line_ = line;
	string delimiter = " ";
	int l = line_.find(delimiter);
	string network = line_.substr(0, l);
	line_ = line_.substr(l+1, line.length());
	l = line_.find(delimiter);
	string word =line_.substr(0,l);
	line_ = line_.substr(l+1,line.length());
	string distance = line_;
	delimiter = "/";
	l = network.find(delimiter);
	string ip = network.substr(0,l);
	string mask = network.substr(l+1,network.length());
    int dist = stoi(distance);
	auto start = std::chrono::high_resolution_clock::now();
	network_attr attr;
	attr.broadcast = broadcast(stoi(mask), ip);
	attr.network_addr = network_addr(stoi(mask), ip);
	attr.mask = stoi(mask);
	attr.dist = dist;
	attr.my_ip = ip;
	attr.time = start;
	route_table.push_back(make_triplet(ip, stoi(mask), dist, ip));
	naighbours.push_back(attr);
}

int main(int argc,char* argv[])

{
	string file_name;
	std::string str;
	std::ifstream file(argv[1]);
	std::getline(file,str);
	int num = stoi(str);
	if (argc < 2){
		fprintf(stderr, "Too few arguments: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
  	while (num > 0){
		std::getline (file,str);
    	prepare(str);
		num--;
  	}
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	 if (sockfd < 0) {
	 	fprintf(stderr, "socket error: %s\n", strerror(errno));
	 	return EXIT_FAILURE;
	}
	int trueflag = 1;
	 if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &trueflag, sizeof trueflag) < 0){
	 	fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
	}

	receive(sockfd);



	return EXIT_SUCCESS;
}

