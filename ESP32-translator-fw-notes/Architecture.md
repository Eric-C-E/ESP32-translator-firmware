running in a venv

Primary problem: Feeding openai-Whisper the information it needs, (waiting) for completion, then feeding it more information. Deconflicting the partial transcripts, and feeding openai-Whisper from a numpy array.

**Expected Data Format**

Numpy arrays feeding Whisper are expected to be of format:
16kHz Mono to bypass FFMPEG invocation.

**Optimizing For Speed**

Whisper only needs to be invoked once. 
