Priority Medium

Repeatedly tries to establish wifi-connection.

If connection drops, tries again.

Does not try if wifi-connection is good.

Sends RSSI every 1 second (such that it can be displayed on the Visor)




**Connects to**

[[AUDIO UPLINK TASK]]
[[TEXT DOWNLINK TASK]]
as wifi-enabling task

[[GRAPHICS TASK]]
as RSSI producer