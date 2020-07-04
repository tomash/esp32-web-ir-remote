# esp32-web-ir-remote

Tiny web infrared remote, to control TV and soundbar with smartphone or computer. Ran on ESP32, adapted from older code I wrote for ESP8266. Not generic, very specific especially in terms of Sony soundbar (HT-CT380).

IR LED is a standard 5mm 940nm LED, driven with an NPN transistor for 80mA current.

## Getting your remote's codes

Connect IR receiver like Vishay's TSOP4838 and check out the gist [ir\_recv\_dump.ino](https://gist.github.com/tomash/9fec25d3be57ab1468ac7823dafd9a83).


