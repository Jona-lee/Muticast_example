Muticast_example
================

An example for Multicast sending and receiving

eg:

#Client: 
send "Hello,World!" to ip group 224.0.1.77 port 12345  
**./mcast -c -g 224.0.1.77 -p 12345** 

#Server:
receive from ip group 224.0.1.77 port 12345 and loop print  
**./mcast -s -g 224.0.1.77 -p 12345**
