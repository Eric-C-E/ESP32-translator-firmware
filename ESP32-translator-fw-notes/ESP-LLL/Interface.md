- magic `0xA17A` (2)
    
- type (1) = AUDIO(1) / TEXT(2) / CTRL(3)
    
- flags (1) = direction / mic slot / screen id
    
- seq (4)
    
- payload_len (4)
    
- reserved/timestamp (4)

TCP header structure:
[https://www.cbtnuggets.com/blog/technology/networking/what-is-tcp-header]


Describes communication between TCP sockets of ESP32-S3 and Jetson Orin Nano

[[AUDIO UPLINK TASK]][[TEXT DOWNLINK TASK]]