#!/usr/bin/env python3
from scapy.all import *

dst = IP(dst="www.scu.edu.cn").dst
print(f'Trace route to {dst}')
for ttl in range(1,129):
    pkt = IP(dst=dst, ttl=ttl)/ICMP()
    ans = sr1(pkt, verbose=0, timeout=1)
    if ans is None:
        print(f' {ttl} timeout')
    elif ans.src == dst:
        print(f' {ttl} {ans.src}')
        break
    else:
        print(f' {ttl} {ans.src}')