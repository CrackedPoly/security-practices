#!/usr/bin/env python3

from scapy.all import *
def print_pkt(pkt):
	pkt.show()
pkt = sniff(iface='enp0s5', filter='net 220.167.42', prn=print_pkt)