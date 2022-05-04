#include <libnet.h>
#include <stdlib.h>
#include <stdio.h>

// 数据包发送函数
void send_packets() {
	int i;
	libnet_t* l = NULL;/* libnet 句柄*/
	libnet_ptag_t protocol_tag;/* 协议标记 */
	char* payload = "12435"; /* 负载 ，内容设 up to you*/
	u_short payload_length = 0; /* 负载长 */
	char* device = "eth0";/*网络设接口*/
	char* destination_ip_str = "8.8.8.8";/* 目的 IP 地址符串 */
	char* source_ip_str = "192.168.1.110"; /* 源 IP 地址符串 */
	u_long source_ip = 0; /* 源 IP 地 */
	u_long destination_ip = 0; /* 目的 IP 地 */
	char errbuf[LIBNET_ERRBUF_SIZE]; /* 错误信 */
	int packet_length; /* 发送的数包的长度 */
	// 具体程序段，ICMP 在 IP 层构建
	l = libnet_init( /* 初始化 libnet */
		LIBNET_RAW4, /* libnet 类型，为原始套接字 IPv4 类型 */
		device, /* 网络设备接口 */
		errbuf/* 错误信息 */
	);
	source_ip = libnet_name2addr4(l, source_ip_str, LIBNET_RESOLVE);
	destination_ip = libnet_name2addr4(l, destination_ip_str, LIBNET_RESOLVE);
	protocol_tag = libnet_build_icmpv4_echo( /* 构造 ICMP 回显据包 */
		ICMP_ECHO, /* 类型，此时回显请求 */
		0,/* 代码，应为 0 */
		0, /* 校验和，为 0，表示 libnet 句柄自动计算 */
		123, /* 标识符，赋值为 1 自己任意填写数值 */
		456, /* ***，赋值为 245，自任意填写数值 */
		NULL, /* 负载，赋为空 */
		0, /* 负载的长度赋值为 0 */
		l, /* libnet 句柄，应该是由 libnet_t()函数得到的 */
		0 /* 协议块标记，赋值为，表示构造一个新的协议块 */
	);
	protocol_tag = libnet_build_ipv4(/* 构造 IP 协块 */
		LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + payload_length,/*
		IP 协议块长度 */
		0, /* 服务质量，里赋值为 0 */
		10, /* 标识符，这赋值为 10 */
		0, /* 偏移，这赋值为 0 */
		20,/* 生存时间，里赋值为 20 */
		IPPROTO_ICMP,/* 上层协议类型，里是 ICMP 协议 */
		0, /* 校验和，这里为 0 表由 libnet 计算校验和 */
		source_ip, /* 源 IP 地 */
		destination_ip,/* 目的 IP 地 */
		payload, /* 负载*/
		payload_length, /* 负载的度 */
		l,/* libnet 句柄*/
		0 /* 协议块标记，为 0 表构造一个新的 IP 协议块 */
	);
	while (1) {
		packet_length = libnet_write(l); /* 发送由 libnet 句柄示的数据包 */
		printf("the length of the ICMP packet is %d\n", packet_length);
		printf("%s", errbuf);
	}
	libnet_destroy(l);
}

void main()
{
	send_packets();
}