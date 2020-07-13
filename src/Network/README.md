# NPeer
A Library that provides an intermediary between the Network and Audio Encoder/Decoder
## How it works
Circles = things provided by NPeer
Rectangles = things NPeer needs to interface with (aka things you need to provide)
```mermaid
graph TD
N{Network} --> C
E -- Send Audio --> N
A -- AudioOutPacket -->E((NPeer Thread))
C[Network Receive] -- Supply AudioInPacket --> A

A((NPeer Library: An Intermediary for Peers Chat)) -- Request AudioInPacket --> B(Audio Encoder)
B -- Supply AudioOutPacket --> A

B -- Send Audio --> D{Mic/Speaker}
D -- Capture Audio --> B
```
