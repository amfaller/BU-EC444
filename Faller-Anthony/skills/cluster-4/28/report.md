#  Leader Election

Author: Anthony Faller

Date: 2020-11-7
-----

## Summary
I have created code for a leader election using some espressif UDP client/server examples and the Bully Algorithm described in class. Upon restart, the FOBs hold an election. They broadcast a message containing their ID, their perceived leader ID, and a flag for whether or not they are a leader. After 10 seconds, the election ends and a leader is chosen. The leader then broadcasts a message (heartbeat) every second, and if a heartbeat is not recevied by the nonleaders within 5 seconds, the election restarts. 

The demo video includes `idf.py -p PORT monitor`, as I did not see a way to show what was happening with the three-fob election without it. 
## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
[UDP Client Example](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client)

[UDP Server Example](https://github.com/espressif/esp-idf/blob/master/examples/protocols/sockets/udp_server)



## Supporting Artifacts
[Link to Demo Video](https://drive.google.com/file/d/1UOoiSMMJQKrJtjlLfs6GOSiiKjg0b8pa/view?usp=sharing)

-----
