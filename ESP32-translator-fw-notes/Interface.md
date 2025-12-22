- magic `0xA17A` (2)
    
- type (1) = AUDIO(1) / TEXT(2) / CTRL(3)
    
- flags (1) = direction / mic slot / screen id
    
- seq (4)
    
- payload_len (4)
    
- reserved/timestamp (4)

TCP header structure:
[https://www.cbtnuggets.com/blog/technology/networking/what-is-tcp-header]



**audio**- owned by [[AUDIO CAPTURE TASK]]

single ringbuffer type RingbufHandle_t audio_rb

**FSM** - owned by [[GPIO TASK]]

enum app_gpio_state_t type app_gpio_state