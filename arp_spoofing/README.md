# ARP Spoofing
In a local network, attacker first broadcasts ARP request to get the victim's MAC address, 
then sends a fabricated ARP reply to the victim with `sender_mac`=attacker's MAC address, 
`target_mac`=victim's MAC address, `sender_ip`=**gateway's IP address**, 
`target_ip`=victim's IP address. 

By doing this, packets the victim intended to
send to gateway will all be forwarded to the attacker, because the victim 
has a item of wrong map(gateway's IP address: attacker's MAC address) in his ARP table.


## How to run
Compile and run `sudo ./arp_spoof IF_NAME VICTIM_IP`

## Result
Here is an snapshot of victim's ARP table. `192.168.1.120` is the IP address of attacker and `192.168.1.1` is the IP address of gateway.

![result](https://raw.githubusercontent.com/CrackedPoly/security-practices/main/arp_spoofing/result.png)
