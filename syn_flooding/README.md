# TCP SYN flooding
Attacker sends a lot of TCP SYN packet to the victim with random IP address. The victim will 
receive TCP SYN packets with various IP address and create a lot of TCP SYN half-open 
sockets.

## Result
Here is an output of victim's `netstat`.

![result](https://raw.githubusercontent.com/CrackedPoly/security-practices/main/syn_flooding/result.png)
