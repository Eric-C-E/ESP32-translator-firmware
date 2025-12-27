```python
import numpy as np

def decode_audio_payload(payload: bytes) -> np.ndarray: #bytes is a CLASS

np.empty((0,), dtype = np.float32)

data = np.frombuffer(payload, dtype = np.uint8).reshape(-1, SAMPLE_BYTES)

vals = data([:,0].astype(np.int32)) | (data[:,1].astype(np.int32)<<8 | (data[:,2].astype(np.int32)<<16))
neg = (vals&0x800000) !=0
vals[neg] -= 1<<24

np.concatenate(destination, source)
```

numpy allows better numerical operations in Python.
With MATLAB-LIKE syntax.

Good for DSP.

