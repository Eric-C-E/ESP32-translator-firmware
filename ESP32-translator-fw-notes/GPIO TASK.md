Priority: High

GPIO Task:
- Read GPIO Events via ISRs
- Debounce
- Keep a finite state machine current.

Connects to:
[[AUDIO CAPTURE TASK]]
with Finite State Machine.

[[AUDIO UPLINK TASK]]
with Finite State Machine.

[[GRAPHICS TASK]]
with Finite State Machine.

Complete 

Can change the GPIO pins used, currently GPIO0, GPIO1.

Can get state anywhere using gpio_get_state

```c
#include "app_gpio_h"

void task(void *arg)
{
	while(1)
	{
		app_gpio_state_t state = gpio_get_state(); //enum
		if (state == APP_GPIO_STATE_TRANSLATE_LANG1){do something};
	}
}
```


