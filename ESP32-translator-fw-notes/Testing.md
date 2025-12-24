Integration testing: Part 1 - Audio Pipeline

log: E (1152805) TCP tx task: TCP tx lang1 state - failed to read from audio rb

State machine is functional.
Display seems to work without hanging - good scheduling.

**Problem 1** 
Problem: reading null values from ringbuffer.

TCP Task is on core 0.

Audio task is on core 1.

Ringbuffer is not filling quickly enough.

Problems spotted: vtaskdelay of 30 ms on Audio task. That is a problem.
Removed, set lower priority for GPIOI. Checked GPIO for any blockers.

Works. 

**Problem 2** : The TCP buffer can sometimes send less than 3072 bytes (expected if from DMA buffer API). It is not possible to change that behavior. We deal with it anyway by appending payload_len in the header struct.

Got PERIODIC spikes in the receiver side. Turns out it was bad parsing (1024/2048 are not multiples of three therefore the parser started aligning mid-frame, causing misalignment when decoded. The fix was to add a carry buffer method to ensure we have mod 6 bytes being appended, and save extra packets for later, to ensure the next packet does not start without the previous packet being mod 6 (else packet might start in middle of frame)

**Problem 3** : Sometimes audio_rb had more than 3072 bytes. Upon retrieval of more than 3072, accessed forbidden memory crashed core 0.

Fixed by using vRingBufferreceiveUpTo() with argument 3072 maximum bytes.

No crashes in a few minutes. Good enough.

Now, it looks like the Jetson Receiver is *reliably* receiving packets without drop from the ESP. 
We can consider the recording pipeline complete.

Integration Testing: Part 2: Display Pipeline