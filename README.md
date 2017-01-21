# Mourino
a MIDI to MIDI-BLE adapter using Arduino 101

This sketch implements the conversion from regular serial MIDI messages received on RX
pin to the Bluetooth Low Energy MIDI Specification as detailed in the docucument at
[https://developer.apple.com/bluetooth/Apple-Bluetooth-Low-Energy-MIDI-Specification.pdf](https://developer.apple.com/bluetooth/Apple-Bluetooth-Low-Energy-MIDI-Specification.pdf)

To connect a MIDI keyboard to the Arduino 101 RX pin you should make a small circuit
using an optocoupler as largely shown on internet (search for images about "MIDI optocoupler
arduino").

## Important
Make sure to update your Arduino 101 firmware and libraries to the version 2.0.0 or above.
Instructions at [https://github.com/01org/corelibs-arduino101](https://github.com/01org/corelibs-arduino101)
