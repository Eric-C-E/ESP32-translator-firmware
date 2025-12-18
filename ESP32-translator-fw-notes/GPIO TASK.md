Priority: High

GPIO Task:
- Read GPIO Events via ISRs
- Debounce
- Keep a finite state machine current.

For bounces occurring as transients, use timer debounce.

(works for spikes too... if edge detected but no )

Connects to:
[[AUDIO CAPTURE TASK]]
with Finite State Machine.

[[AUDIO UPLINK TASK]]
with Finite State Machine.



