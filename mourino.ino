/*
Mourino: a MIDI to MIDI-BLE adapter using Arduino 101, based on MIDIBLE.ino example
Copyright (C) 2017 Daniel Moura <oxe@oxesoft.com>

This sketch implements the conversion between standard serial MIDI messages and
Bluetooth Low Energy MIDI Specification as detailed in the docucument at
https://developer.apple.com/bluetooth/Apple-Bluetooth-Low-Energy-MIDI-Specification.pdf

To connect a MIDI keyboard to the Arduino 101 RX pin you should make a small circuit
using an optocoupler as largely shown on internet (make a search of images with the words
"MIDI optocoupler arduino"). To connect the Arduino 101 TX pin to your MIDI device just
use a 220R resistor.

This code is originally hosted at https://github.com/oxesoft/mourino

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <CurieBLE.h>

#define DEVICE_NAME                 "Mourino"
#define MINIMUM_CONNECTION_INTERVAL 11250 // 11.25 ms
#define SINGLE_BLE_PACKET_LENGTH    BLE_MAX_ATTR_DATA_LEN
#define MIDI_EVENT_MAX_LENGTH       SINGLE_BLE_PACKET_LENGTH - 1 /* minus the header size */

#define EXTERNAL_LED_PIN            7

BLEService midiService("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
BLECharacteristic midiChar("7772E5DB-3868-4112-A1A9-F2669D106BF3", BLEWrite | BLEWriteWithoutResponse | BLENotify | BLERead, SINGLE_BLE_PACKET_LENGTH);

// variables used in SERIAL to BLE only
uint8_t midiData[SINGLE_BLE_PACKET_LENGTH];
uint8_t midiEvent[MIDI_EVENT_MAX_LENGTH];
int midiDataPosition;
int midiEventPosition;
byte prevStatus;

bool isConnected;

void sendPacket() {
    midiChar.setValue(midiData, midiDataPosition);
    midiDataPosition = 1;
    prevStatus = 0;
}

void appendEvent(byte *event, int len) {
    if (midiDataPosition + len > SINGLE_BLE_PACKET_LENGTH) {
        sendPacket();
    }
    memcpy(midiData + midiDataPosition, event, len);
    midiDataPosition += len;
    midiEventPosition = 0;
}

void addEventByte(byte b) {
    midiEvent[midiEventPosition++] = b;
    if (midiEventPosition >= MIDI_EVENT_MAX_LENGTH) { // sysex data
        appendEvent(midiEvent, midiEventPosition);
    }
}

int getEventSize(byte b) {
    /*
     * https://www.midi.org/specifications/item/table-1-summary-of-midi-message
     */
    byte eventType = b & 0xF0;
    switch (eventType) {
    case 0x80:
    case 0x90:
    case 0xA0:
    case 0xB0:
    case 0xE0:
        return 2;
    case 0xC0:
    case 0xD0:
        return 1;
    case 0xF0:
        switch (b) {
        case 0xF0:
            return -1;
        case 0xF1:
            return 1;
        case 0xF2:
            return 2;
        case 0xF3:
            return 1;
        }
    }
    return 0;
}

void processByte(byte b) {
    static int midiEventCount = 0;
    bool newEvent = b & 0x80;
    if (newEvent) {
        if (b == 0xF7) {
            appendEvent(midiEvent, midiEventPosition);
        }
        midiEventPosition = 0;
        midiEventCount = getEventSize(b);
        if (prevStatus != b || midiDataPosition + midiEventCount > SINGLE_BLE_PACKET_LENGTH) {
            addEventByte(0x80); // empty timestamp
            addEventByte(b);
        }
        if (midiEventCount > 0) {
            prevStatus = b;
        } else {
            prevStatus = 0;
        }
    } else {
        if (midiEventCount > 0) {
            midiEventCount--;
        }
        addEventByte(b);
    }
    if (midiEventCount == 0) {
        appendEvent(midiEvent, midiEventPosition);
    }
}

void midiCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
    static char outputState     = 1;
    static char eventBytesCount = 0;
    static byte currentStatus   = 0;
    int   len    = characteristic.valueLength();
    byte *buffer = (byte*)characteristic.value();
    if (len < 0 || buffer == NULL) {
        return;
    }
    len--; buffer++; // ignore header byte
    while (len--) {
        byte b = *(buffer++);
        switch (outputState) {
        case 1: // timestamp byte
            if (b & 0x80) {
                outputState = 2;
            } else {
                eventBytesCount = getEventSize(currentStatus);
                Serial1.write(currentStatus);
                Serial1.write(b);
                eventBytesCount--;
                if (eventBytesCount) {
                    outputState = 3;
                } else {
                    outputState = 1;
                }
            }
            break;
        case 2: // event start
            if (b & 0x80) {
                eventBytesCount = getEventSize(b);
                currentStatus = b;
                Serial1.write(b);
            } else {
                eventBytesCount = getEventSize(currentStatus);
                Serial1.write(currentStatus);
                Serial1.write(b);
                eventBytesCount--;
            }
            if (eventBytesCount) {
                outputState = 3;
            } else {
                outputState = 1;
            }
            break;
        case 3: // rest of event
            if (b & 0x80) { // timestamp byte
                outputState = 2;
            } else {
                Serial1.write(b);
                if (eventBytesCount >= 0) {
                    eventBytesCount--;
                }
                if (eventBytesCount == 0) {
                    outputState = 1;
                }
            }
            break;
        }
    }
}

void midiDeviceConnectHandler(BLEDevice central) {
    isConnected = true;
}

void midiDeviceDisconnectHandler(BLEDevice central) {
    isConnected = false;
}

void ledTick() {
    static unsigned long ledTime = 0;
    static byte ledValue = HIGH;
    const int LED_BLINK_TIME = 500;
    if (isConnected) {
        digitalWrite(EXTERNAL_LED_PIN, HIGH);
        return;
    }
    if (millis() - ledTime > LED_BLINK_TIME) {
        ledValue = !ledValue;
        digitalWrite(EXTERNAL_LED_PIN, ledValue);
        ledTime = millis();
    }
}

void setup() {
    midiData[0] = 0x80; // header with empty timestamp
    midiDataPosition = 1;
    prevStatus = 0x00;
    isConnected = false;

    pinMode(EXTERNAL_LED_PIN, OUTPUT);
    digitalWrite(EXTERNAL_LED_PIN, HIGH);
    
    Serial1.begin(31250);    

    BLE.begin();    
    BLE.setLocalName(DEVICE_NAME);
    BLE.setDeviceName(DEVICE_NAME);
    BLE.setAdvertisedServiceUuid(midiService.uuid());
    midiService.addCharacteristic(midiChar);
    BLE.addService(midiService);
    BLE.setEventHandler(BLEConnected, midiDeviceConnectHandler);
    BLE.setEventHandler(BLEDisconnected, midiDeviceDisconnectHandler);
    midiChar.setEventHandler(BLEWritten, midiCharacteristicWritten);
    BLE.advertise();
}

void loop() {
    static unsigned long time = 0;

    if (Serial1.available()) {
        byte b = Serial1.read();
        processByte(b);
    }

    if (micros() - time >= MINIMUM_CONNECTION_INTERVAL) {
        if (midiDataPosition > 1) {
            sendPacket();
        }
        time = micros();
        BLE.poll();
        ledTick();
    }
}
