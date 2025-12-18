Priority: High

GPIO Task:
- Reads I2S stereo into I2S mono 16kHz Little Endian PCM format.
- Writes to ringbuffer (audio ringbuffer) only if on capture mode.
- No networking functionality
- Have to figure out deconflicting of which language is currently sent and what's the in the ringbuffer. Eg. Don't send data containing english that's remaining in buffer if the current command is French. 

If the I2S mic outputs 32 bit words, we can downshift to int16.
We have 24 bit output (probably 32 bit)
Can shift like int16 = (sample32 >> 14)


Connects to:
[[GPIO TASK]]
with Finite State Machine.

[[AUDIO UPLINK TASK]]
with Audio Ringbuffer

