/*
Mourino: a MIDI to MIDI-BLE adapter using Arduino 101, based on MIDIBLE.ino example
Copyright (C) 2017 Daniel Moura <oxe@oxesoft.com>

This sketch implements the conversion from regular serial MIDI messages received on RX
pin to the Bluetooth Low Energy MIDI Specification as detailed in the docucument at
https://developer.apple.com/bluetooth/Apple-Bluetooth-Low-Energy-MIDI-Specification.pdf

To connect a MIDI keyboard to the Arduino 101 RX pin you should make a small circuit
using an optocoupler as largely shown on internet (make a search of images with the words
"MIDI optocoupler arduino").

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

#define MINIMUM_CONNECTION_INTERVAL 11250 // 11.25 ms
#define SINGLE_BLE_PACKET_LENGTH    BLE_MAX_ATTR_DATA_LEN
#define MIDI_EVENT_MAX_LENGTH       SINGLE_BLE_PACKET_LENGTH - 1 /* minus the header size */
#define EMPTY_TIMESTAMP             0x80

BLEService midiService("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
BLECharacteristic midiChar("7772E5DB-3868-4112-A1A9-F2669D106BF3", BLEWrite | BLEWriteWithoutResponse | BLENotify, SINGLE_BLE_PACKET_LENGTH);

uint8_t midiData[SINGLE_BLE_PACKET_LENGTH];
uint8_t midiEvent[MIDI_EVENT_MAX_LENGTH];
int midiDataPosition;
int midiEventPosition;
unsigned long time;
byte prevStatus;
int midiEventCount;

void midiDeviceConnectHandler(BLEDevice central) {
    // central connected event handler
    Serial.print("Connected event, central: ");
    Serial.println(central.address());
}

void midiDeviceDisconnectHandler(BLEDevice central) {
    // central disconnected event handler
    Serial.print("Disconnected event, central: ");
    Serial.println(central.address());
}

void BLESetup()
{
    BLE.begin();
    
    // set the local name peripheral advertises
    BLE.setLocalName("Mourino");
    BLE.setDeviceName("Mourino");
    
    // set the UUID for the service this peripheral advertises
    BLE.setAdvertisedServiceUuid(midiService.uuid());
    
    // add service and characteristic
    midiService.addCharacteristic(midiChar);
    BLE.addService(midiService);
    
    // assign event handlers for connected, disconnected to peripheral
    BLE.setEventHandler(BLEConnected, midiDeviceConnectHandler);
    BLE.setEventHandler(BLEDisconnected, midiDeviceDisconnectHandler);
    
    // set an initial value for the characteristic
    midiChar.setValue(midiData, 5);
    
    // advertise the service
    BLE.advertise();
}

void sendPacket() {
    midiChar.setValue(midiData, midiDataPosition);
    midiDataPosition = 1;
    prevStatus = 0;
}

void appendEvent(byte *event, int len) {
    if (midiDataPosition + len >= SINGLE_BLE_PACKET_LENGTH) {
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

void processByte(byte b) {
    bool newEvent = b & 0x80;
    if (newEvent) {
        /*
         * https://www.midi.org/specifications/item/table-1-summary-of-midi-message
         */
        if (b == 0xF7) {
            appendEvent(midiEvent, midiEventPosition);
        }
        midiEventPosition = 0;
        byte eventType = b & 0xF0;
        switch (eventType) {
            case 0x80:
            case 0x90:
            case 0xA0:
            case 0xB0:
            case 0xE0:
                midiEventCount = 2;
                break;
            case 0xC0:
            case 0xD0:
                midiEventCount = 1;
                break;
            case 0xF0:
                switch (b) {
                    case 0xF0:
                        midiEventCount = -1;
                        break;
                    case 0xF1:
                        midiEventCount = 1;
                        break;
                    case 0xF2:
                        midiEventCount = 2;
                        break;
                    case 0xF3:
                        midiEventCount = 1;
                        break;
                    default:
                        midiEventCount = 0;
                        break;
                }
                break;
        }
        if (prevStatus != b || midiDataPosition + midiEventCount >= SINGLE_BLE_PACKET_LENGTH) {
            addEventByte(EMPTY_TIMESTAMP);
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

void setup() {
    midiData[0] = 0x80; // header with empty timestamp
    midiDataPosition = 1;
    prevStatus = 0x00;
    midiEventCount = 0;
    time = 0;
    Serial.begin(115200);
    while (!Serial);
    Serial1.begin(31250);    
    while (!Serial1);
    BLESetup();
    Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
    if (Serial1.available()) {
        byte b = Serial1.read();
        processByte(b);
    }

    if (micros() - time >= MINIMUM_CONNECTION_INTERVAL) {
        if (midiDataPosition > 1) {
            sendPacket();
        }
        time = micros();
    }
}

