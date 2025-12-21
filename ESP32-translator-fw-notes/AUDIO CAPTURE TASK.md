Priority: High

Connects to:
[[GPIO TASK]]
with Finite State Machine.

[[AUDIO UPLINK TASK]]
with Audio Ringbuffer

**Architecture** from [[AUDIO Learning]]

Microphone currently records 16kHz stereo format, 24 bit per "slot".

This program won't do any conversion of audio on its own, but on Jetson Side, 

Numpy array fed to Whisper will be mono, 16kHz.
Which means we will need to make the stereo mono, which numpy probably has a function for.

Ringbuffer audio_rb is currently 32768 bytes (32kB) which means it is capable of handling a few hundred ms of TCP hangup. The TCP sockets running on core 1 shouldn't ever reach that level of delay. Microphone writes to DMA, which is checked by the audio task, and buffered into audio_rb at the exact amount of bytes received from the DMA buffer (most likely allocated heap amount 3072 bytes, due to 500 ms wait, but it should hit that in ~30ms).

Writes into a ring-buffer for the TCP transmit socket!
