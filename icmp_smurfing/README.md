# ICMP Smurfing
Attacker sends malicious ICMP Pings to some hosts in the Internet. And the 'src_ip' 
field of the malicious packets is replaced with victim's IP address. If the attacker 
sends a lots in a short time, the victim will be flooded by ICMP Pong traffic.(DDoS)

## Result
Here is an output of victim's `tcpdump`.

![result](https://raw.githubusercontent.com/CrackedPoly/security-practices/main/icmp_smurfing/result.png)
