DiffServ-and-TCP-Congestion-Control-NS3-simulation


                                        +--n10
                                        | 
         n0--+            +--n6         +---n11
             |            |             |
             n2 ---n3 --- n5 --- n7 --- n9---n12
             |     |             |      |
         n1--+     +--n4         +--n8  +---n13
                                        |
                                        +--n14


Use  NS-3 Simulator  to setup five TCP flows (A to A, B to B, etc.) in the above parking-lot configuration. Each physical link is assumed to have a bandwidth of 10 Mbps, and each link has propagation delay 100 us.

Assume A, B, C, D,  and E generate Poisson traffic, respectively. Each TCP flow has
       (1) Mean packet generation rate = 2 Mbps. 
       (2) Max IP packet length = 1400 bytes.
       (3) Queuing discipline in routers: FIFO with Drop Tail. 
       (4) Buffer size = adjustable up to 5 packets.   
ECN: Once congestion occurs in a router, the ECN field in IP header is set. The ECE (ECN Echo) bit in TCP header is set by the receiver on every ACK packet until the CWR flag is received

TCP Parameters: 
       (1) TCP-Reno is employed.
       (2) Max. Segment Size (MSS) = 1000 bytes.
       (3) Segment Processing Time = 1 msec.   
       (4) Receiver’s Window  Size = 20 Segments. 
       (5) Enable Slow Start: Initial cwnd  = 1 Segment.
       
In the Simulation:
        (1) (10%) Show  the buffer occupancy (i.e., queue length) for every router vs simulation time.
        (2) (10%) Show the throughput (Mbps) for every TCP flow vs router buffer size.
        (3) (10%) Show the variations of cwnd for every TCP flow vs simulation time.
        (4) (10%) Show packet loss rate in every router by assuming router’s buffer size = 1 packet.
        (5) (10%) Assume RED is enabled. Discuss on the improvements with and without enabling RED.   
             Assume buffer size = 4 packets. Set up THmin = 1  packet and TH max = 3 packets.
