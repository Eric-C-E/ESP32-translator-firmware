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

Payload Structure:
Up to 3072 bytes of audio bytes. Packed. No padding. Little endian, stereo interleaved format.
L0 | R0 | L1 | R1 | etc...
Bytes grouped in 3-byte samples, 6 bytes per stereo frame.

Have to reshape into Nx3 bytes. 

Normalize for WHISPER: float32 in [-1.0, +1.0] by dividing by 2**23, mono. 
16 kHz is already expected.
Ensure payload_len is a multiple of six before sending to whisper. %6.

