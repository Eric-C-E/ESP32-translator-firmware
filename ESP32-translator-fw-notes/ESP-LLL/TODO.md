Days of work: add up to 1

*Refactor display code to be 2 screen friendly 0.2 - DONE*

*Create the gpio task and monitor states in monitor 0.4 - done*

*Create TCP task that reads audio_rb that sends continually to Jetson AP 0.4
Test this task using NetCat.
Subtasks - write TCP program
*- update menuconfig Kconfig file to remove ipv6, set defaults, change names*

Next Day

Actually connect the microphone and see if those packets are any useful using a simple numpy test program. 0.2

if not good, fix, delay

if good, work on the display task -> make it do what we want for circular screen instead of the ball 0.3 test on synthetic data

*By this point, 4 tasks are done, only TCP socket for rx needs to be done- DONE*

Next Day

Create the plumbing between TCP RX socket and Display socket 0.5

Full System verification test and allocated buffer for delays
Ideal state: mic works, get workable audio on Jetson
netcat Jetson sends packets, get printed on screen.

Next Day

*Build hardware and full system integrated verification test 1.0 - DONE

Pivot to Jetson Work - Out of Scope.

above: 4 days of work.
ESP32 Total: ~ 7 days of work
Jetson allocation: 5 days of work.

Mechanical Design - 1 day of work
Electrical PCB Design - 1 day of work (v1)
Final Assembly - 1 day of work

