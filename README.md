# Mourino
MIDI over Bluetooth Low Energy (BLE) using an Arduino 101

This sketch implements the conversion between standard serial MIDI messages and
Bluetooth Low Energy MIDI Specification as detailed in the document at
[https://www.midi.org/specifications/midi-transports-specifications/midi-over-bluetooth-low-energy-ble-midi](https://www.midi.org/specifications/midi-transports-specifications/midi-over-bluetooth-low-energy-ble-midi)

![mourino protoboard](https://raw.githubusercontent.com/oxesoft/mourino/master/mourino.jpg)

![mourino boxed](https://raw.githubusercontent.com/oxesoft/mourino/master/boxed.jpg)

To connect a MIDI keyboard to the Arduino 101 RX pin you should make a small circuit
using an optocoupler as largely shown on internet (make a search of images with the words
"MIDI optocoupler arduino"). To connect the Arduino 101 TX pin to your MIDI device just
use a 220R resistor.

## Important
Make sure to update your Arduino 101 firmware and libraries to the version 2.0.0 or above.
Instructions at [http://forum.arduino.cc/index.php?topic=443728.0](http://forum.arduino.cc/index.php?topic=443728.0)
