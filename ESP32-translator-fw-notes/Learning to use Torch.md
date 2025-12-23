Original plan was to use acceleration with TensorRT, namely whisperTRT. 

Later found out whisperTRT is too narrow-scope and only works for english.

Reverted back to normal Torch openai-whisper. Many issues when attempting to run it, stemming from Torch version (no kernel). Torch must support NVidia AMPERE architecture, and be made specifically for Jetpack - this is now installed.

Numpy 2+ is NOT compatible with this version of torch. Installed a downgraded Numpy - works.

Used several test phrases to test the model. GPU is faster, CPU is slow. Whisper needs context to work properly, so a good sliding window method will need to be architected.