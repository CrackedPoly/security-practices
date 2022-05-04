#!/usr/bin/env python3
from scapy.all import *
import _thread

def spoof():
	a = IP()
	b = ICMP()
	p = a/b
	p.show()
	p.dst = '10.9.0.5'
	send(p)


def print_pkt(pkt):
	pkt.show()


_thread.start_new_thread(spoof, ())
pkt = sniff(iface='br-04dcb0b4200e', filter='icmp', prn=print_pkt)