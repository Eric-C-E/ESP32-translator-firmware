Priority: Medium

GPIO Task:
- Sends raw stereo audio in 3072 byte chunks from audio ringbuffer
- Packetizes with 8 byte header that contains length information in network protocol format
- IMPORTANT: need to decode from network protocol format on Jetson Side
- Uses FSM state to determine header structure.
- Uses a send_all routine to ensure sending all bytes read from ringbuffer on events outside of program control like a TCP hangup/TCP exit buffer full condition
- TCP Send to Jetson over Wifi-STA

Connects to:
[[GPIO TASK]]
with Finite State Machine.

[[AUDIO UPLINK TASK]]
with Audio Ringbuffer audio_rb

References
[[Interface]]

Testing Audio Uplink using netcat:

Results:

