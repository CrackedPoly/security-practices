#!/usr/bin/env python3

from scapy.all import *
def print_pkt(pkt):
	pkt.show()
pkt = sniff(iface='br-04dcb0b4200e', filter='icmp', prn=print_pkt)