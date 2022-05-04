#include<pcap.h>
#include<stdio.h>
#include<arpa/inet.h>
#include "myheader.h"
#include<string.h>

unsigned short in_cksum(unsigned short* buf, int length)
{
	unsigned short* w = buf;
	int nleft = length;
	int sum = 0;
	unsigned short temp = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(u_char*)(&temp) = *(u_char*)w;
		sum += temp;
	}

	sum = (sum >> 16) + (sum & 0xffff);  // add hi 16 to low 16
	sum += (sum >> 16);                  // add carry
	return (unsigned short)(~sum);
}

void send_raw_ip_packet(struct ipheader* ip)
{
	struct sockaddr_in dest_info;
	int enable = 1;

	// Step 1: Create a raw network socket.
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	printf("sock: %d\n", sock);

	// Step 2: Set socket option.
	setsockopt(sock, IPPROTO_IP, IP_HDRINCL,
		&enable, sizeof(enable));

	// Step 3: Provide needed information about destination.
	dest_info.sin_family = AF_INET;
	dest_info.sin_addr = ip->iph_destip;

	// Step 4: Send the packet out.
	sendto(sock, ip, ntohs(ip->iph_len), 0,
		(struct sockaddr*)&dest_info, sizeof(dest_info));
	close(sock);
}

void got_packet(u_char* args, const struct pcap_pkthdr* header,
	const u_char* packet)
{
	struct ethheader* eth = (struct ethheader*)packet;

	if (ntohs(eth->ether_type) == 0x0800) { // 0x0800 is IP type
		struct ipheader* ip = (struct ipheader*)
			(packet + sizeof(struct ethheader));

		printf("From: %s ", inet_ntoa(ip->iph_sourceip));
		printf("To: %s ", inet_ntoa(ip->iph_destip));
		if (ip->iph_protocol == IPPROTO_ICMP)
			printf("protocal: ICMP\n");
		else
			printf("protocal: Others\n");

		struct icmpheader* icmp_pkt = (struct icmpheader*)(packet + sizeof(struct ethheader)
			+ sizeof(struct ipheader));

		if (ip->iph_protocol == IPPROTO_ICMP) {

			char buffer[1500];
			memset(buffer, 0, 1500);

			struct icmpheader* icmp = (struct icmpheader*)
				(buffer + sizeof(struct ipheader));
			icmp->icmp_type = 0; //ICMP Type: 8 is request, 0 is reply.
			icmp->icmp_code = 0;
			icmp->icmp_id = icmp_pkt->icmp_id;
			icmp->icmp_seq = icmp_pkt->icmp_seq;
			printf("icmp id: %d, seq: %d\n", ntohs(icmp_pkt->icmp_id), ntohs(icmp_pkt->icmp_seq));

			// Calculate the checksum for integrity
			icmp->icmp_chksum = 0;
			icmp->icmp_chksum = in_cksum((unsigned short*)icmp,
				sizeof(struct icmpheader));

			struct ipheader* ipp = (struct ipheader*)buffer;
			ipp->iph_ver = 4;
			ipp->iph_ihl = 5;
			ipp->iph_ttl = 64;
			ipp->iph_sourceip.s_addr = ip->iph_destip.s_addr;
			ipp->iph_destip.s_addr = ip->iph_sourceip.s_addr;
			ipp->iph_protocol = IPPROTO_ICMP;
			ipp->iph_len = htons(sizeof(struct ipheader) +
				sizeof(struct icmpheader));
			printf("send tt source :%s\n", inet_ntoa(ipp->iph_sourceip));
			printf("send tt dest: %s\n", inet_ntoa(ipp->iph_destip));

			send_raw_ip_packet(ipp);

		}
	}
}

int main()
{
	pcap_t* handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter_exp[] = "icmp[icmptype]==icmp-echo";
	bpf_u_int32 net;

	// Step 1: Open live pcap session on NIC with name enp0s5
	handle = pcap_open_live("br-04dcb0b4200e", BUFSIZ, 1, 1000, errbuf);
	printf("listening on network card, ret: %p...\n", handle);

	// Step 2: Compile filter_exp into BPF psuedo-code
	printf("try to compile filter...\n");
	pcap_compile(handle, &fp, filter_exp, 0, net);
	printf("try to set filter...\n");
	pcap_setfilter(handle, &fp);

	// Step 3: Capture packets
	printf("start to sniff...\n");
	pcap_loop(handle, -1, got_packet, NULL);

	pcap_close(handle);   //Close the handle
	return 0;
}

