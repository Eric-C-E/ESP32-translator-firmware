Priority: Medium

GPIO Task:
- Pull correct audio for the given language active state as per GPIO TASK
- Packetize
- Add TCP framing that conveys all necessary information
- TCP Send to Jetson over Wifi-STA

Connects to:
[[GPIO TASK]]
with Finite State Machine.

[[AUDIO UPLINK TASK]]
with Audio Ringbuffer audio_rb

References
[[Interface]]

Testing Audio Uplink

