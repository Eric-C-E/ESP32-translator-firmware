Priority: Medium

GPIO Task:
- TCP Receive Text Packets from Jetson via Wifi-STA
- Uses a recv_all helper function to avoid partial packet reads
- parses header to determine queue to push payload into
- Pushes text payload to queue
- uses the same socket fd as the transmit task, waits on socket ready from transmit task to avoid using an invalid descriptor

Connects to:
[[GRAPHICS TASK]]
with Queue