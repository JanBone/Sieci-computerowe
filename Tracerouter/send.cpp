// Katsiaryna Yalovik 306460 

#include "send.h"


u_int16_t Send::compute_icmp_checksum (const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t* ptr = (const u_int16_t*)buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

icmp Send::MakeHeader(int id){
  struct icmp header;
  header.icmp_type = ICMP_ECHO;
  header.icmp_code = 0;
  header.icmp_hun.ih_idseq.icd_id = getpid(); 
  header.icmp_hun.ih_idseq.icd_seq = id;
  header.icmp_cksum = 0;
  header.icmp_cksum = compute_icmp_checksum((u_int16_t*)&header, sizeof(header));
  return header;
}

void Send::sendPacket(int sockfd,  char* ip_addr, int ttl, int id) {
  int set_s, bytestosend;
  struct sockaddr_in recipient;
  bzero (&recipient, sizeof(recipient));
  recipient.sin_family = AF_INET;
  inet_pton(AF_INET, ip_addr, &(recipient.sin_addr));
  icmp header = MakeHeader(id);
  set_s = setsockopt (sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));  
  if (set_s!=0){
    throw std::invalid_argument( "Setting ttl failed");
  }
  bytestosend = sendto (sockfd , &header, sizeof(header), 0, (struct sockaddr*) &recipient, sizeof(recipient));
  if (bytestosend <=0){
    throw std::invalid_argument( "Sending packet failed");
  }
  
}

 
