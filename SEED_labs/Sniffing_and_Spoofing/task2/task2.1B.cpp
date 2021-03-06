#include <cstdio>
#include <pcap.h>
#include <cstdlib>
#include <ctime>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

void print_mac(u_char mac[6]) {
	for (int i = 0;i < 6;i++) {
		printf("%.2x", mac[i]);
		printf(i == 5 ? "\n" : ":");
	}
}

void print_ip(u_char ip[4]) {
	for (int i = 0;i < 4;i++) {
		printf("%d", ip[i]);
		printf(i == 3 ? "\n" : ".");
	}
}

void print_data(const u_char* payload, uint16_t len) {
	for (int i = 0;i < len;i++) {
		if (i % 16 == 0) {
			printf("\n%04d\t", i);
		}
		printf("%02x ", payload[i]);
	}
	printf("\n");
	for (int i = 0;i < len;i++) {
		if (i % 16 == 0) {
			printf("\n%04d\t", i);
		}
		if (payload[i] >= 32 && payload[i] <= 126) {
			putchar(payload[i]);
		}
		else {
			putchar('.');
		}
	}
	printf("\n");
}

void callback(u_char* userarg, const pcap_pkthdr* pkthdr, const u_char* packet) {
	static uint64_t packet_num = 0;
	ether_header* eth;
	ip* ip_hdr;
	tcphdr* tcp_hdr;
	udphdr* udp_hdr;
	uint16_t payload_len;

	printf("----------------Recv packet----------------\n");
	printf("Recv at %s", ctime((const time_t*)&(pkthdr->ts).tv_sec));
	printf("Packet number: %lu\n", ++packet_num);
	printf("Packet length: %d\n", pkthdr->len);

	eth = (ether_header*)packet;
	printf("Source MAC address: ");
	print_mac(eth->ether_shost);
	printf("Destination MAC address: ");
	print_mac(eth->ether_dhost);

	uint16_t typeno = ntohs(eth->ether_type);
	printf("Network layer protocal:");
	switch (typeno) {
	case ETHERTYPE_IP:
		printf("IPV4\n");
		break;
	case ETHERTYPE_PUP:
		printf("PUP\n");
		break;
	case ETHERTYPE_ARP:
		printf("ARP\n");
		break;
	default:
		printf("UNKNOWN\n");
	}
	if (typeno == ETHERTYPE_IP) {
		ip_hdr = (ip*)(packet + sizeof(ether_header));
		printf("Source IP address: ");
		print_ip((u_char*)&(ip_hdr->ip_src));
		printf("Destination IP address: ");
		print_ip((u_char*)&(ip_hdr->ip_dst));

		printf("Transport layer protocal:");
		typeno = ip_hdr->ip_p;
		if (typeno == IPPROTO_TCP) {
			printf("TCP\n");
			tcp_hdr = (tcphdr*)(packet + sizeof(ether_header) + sizeof(ip));
			printf("Source port: %d\n", ntohs(tcp_hdr->source));
			printf("Dest port: %d\n", ntohs(tcp_hdr->dest));
			payload_len = pkthdr->len - (sizeof(ether_header) + sizeof(ip));
			printf("Payload(%d bytes): \n", payload_len);
			// print_data(packet + (sizeof(ether_header) + sizeof(ip)), payload_len);
		}
		else if (typeno == IPPROTO_UDP) {
			printf("UDP\n");
			udp_hdr = (udphdr*)(packet + sizeof(ether_header) + sizeof(ip));
			printf("Source port: %d\n", ntohs(udp_hdr->source));
			printf("Dest port: %d\n", ntohs(udp_hdr->dest));
			payload_len = pkthdr->len - (sizeof(ether_header) + sizeof(ip));
			printf("Payload(%d bytes): \n", payload_len);
			// print_data(packet + (sizeof(ether_header) + sizeof(ip)), payload_len);
		}
		else if (typeno == IPPROTO_ICMP) {
			printf("ICMP\n");
		}
		else if (typeno == IPPROTO_IGMP) {
			printf("IGMP\n");
		}
		else {
			printf("UNKNOWN\n");
		}
	}
}

int main() {
	pcap_if_t* alldevs;     // ??????????????????
	pcap_if_t dev;          // ????????????
	bpf_u_int32 net = 0;    // ??????ip
	bpf_u_int32 mask = 0;   // ????????????
	pcap_t* handle;         // libpcap??????????????????
	bpf_program fp;         // ?????????
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	in_addr addr;

	handle = pcap_open_live("br-04dcb0b4200e", BUFSIZ, 0, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev.name, errbuf);
		return -1;
	}

	// "tcp and dst portrange 10-100"
	if (pcap_compile(handle, &fp, "icmp and host 10.9.0.5 and host 103.235.46.39", 1, mask) == -1) {
		fprintf(stderr, "Couldn't parse filter ip: %s\n", pcap_geterr(handle));
		return -1;
	}

	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter ip: %s\n", pcap_geterr(handle));
		return -1;
	}

	if (pcap_loop(handle, -1, (pcap_handler)callback, NULL) < 0) {
		fprintf(stderr, "pcap_loop error: %s\n", pcap_geterr(handle));
		exit(0);
	}
	pcap_close(handle);
}