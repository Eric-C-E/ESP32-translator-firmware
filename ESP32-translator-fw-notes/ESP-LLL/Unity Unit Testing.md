Learning with [[AUDIO Learning]][[LVGL Learning]][[Folder Structure]]

Unity is a ESP-IDF built-in unit testing framework. 

It MUST be included with (non-priv) REQUIRES keyword in CMakeLists.

then include

```c
#include "unity.h"
#include "unity_test_runner.h"
```

in each test file, and the test file (any file containing tests) needs the header for the C modules
that are to be tested.

Example Usage: 

Testing of display:

Put the test_cases in main.c, then 
add a header file that exposes everything

Problem: WDT trips. -> fix is to disable WDT on test builds

Mem Leak test yields 7000 byte difference in persistent allocations. This is failed but it's probably healthy.