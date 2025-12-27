running in a venv

Primary problem: Feeding openai-Whisper the information it needs, (waiting) for completion, then feeding it more information. Deconflicting the partial transcripts, and feeding openai-Whisper from a numpy array.

Language: English Speakers speak at about 120-150 WPM. This is around 2-2.5 words per second. 
We can set a *preliminary* speech update frequency to be 2 Hz - 1 update per 500 ms.

**Expected Data Format**

Numpy arrays feeding Whisper are expected to be of format:
16kHz Mono to bypass FFMPEG invocation.

**Optimizing For Speed**

Whisper only needs to be invoked once. 

Torch now has an option called torch.compile to quicken PyTorch code by JIT-compiling PyTorch code into optimized kernels with minal code changes.

Available in Torch 2.0 or later. Deps torch, numpy, scipy.

**Language Translation Model**
Using Ctranslate2 for its robust Python API for efficient transformer models.
Comes with many built-in optimization techniques.
MarianMT models (opus-MT) are 298 MB. 1000 are available. Can get them as it's required.


**Concurrency**


