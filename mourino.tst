/*
Mourino: a MIDI to MIDI-BLE adapter using Arduino 101, based on MIDIBLE.ino example
Copyright (C) 2017 Daniel Moura <oxe@oxesoft.com>

This test simulates the normal operation of a MIDI device connected to the 
Arduino 101, in both ways (IN, OUT)

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

#define byte unsigned char
#define BLE_MAX_ATTR_DATA_LEN   20
#define MINIMUM_CONN_INTERVAL   11250
#define BLEWrite                1
#define BLEWriteWithoutResponse 2
#define BLENotify               4
#define BLERead                 8
#define BLEWritten              1
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define SERIAL_MIDI_BYTES_COUNT 200
byte testStream[SERIAL_MIDI_BYTES_COUNT] = {
    0x90, 0x01, 0x7F,

    0x90, 0x02, 0x01,
    0x90, 0x02, 0x7F,

    0x90, 0x03, 0x01,
    0x90, 0x03, 0x02,
    0x90, 0x03, 0x03,
    0x90, 0x03, 0x04,
    0x90, 0x03, 0x05,
    0x90, 0x03, 0x06,
    0x90, 0x03, 0x07,
    0x90, 0x03, 0x7F,

    0x90, 0x03, 0x01,
    0x91, 0x03, 0x02,
    0x90, 0x03, 0x03,
    0x90, 0x03, 0x04,
    0x90, 0x03, 0x05,
    0x90, 0x03, 0x7F,

    0xC0, 0x7F,

    0xC0, 0x01,
    0xD0, 0x7F,

    0xC0, 0x01,
    0xC0, 0x02,
    0xD0, 0x01,
    0xD0, 0x7F,

    0xC0, 0x01,
    0xC0, 0x02,
    0xD1, 0x01,
    0xD0, 0x7F,

    0x90, 0x01, 0x01,
    0x91, 0x02, 0x02,
    0x93, 0x03, 0x03,
    0x94, 0x04, 0x04,
    0x95, 0x05, 0x05,

    0x96, 0x01, 0x7F,

    0x90, 0x01, 0x01,
    0xC0, 0x01,
    0xC0, 0x02,
    0x91, 0x01, 0x7F,

    0xC0, 0x01,
    0xC0, 0x02,
    0xC0, 0x03,
    0xC0, 0x04,
    0xC0, 0x05,
    0xC0, 0x06,
    0xC0, 0x07,
    0xC0, 0x08,
    0xC0, 0x09,
    0xC0, 0x0A,
    0xC0, 0x0B,
    0xC0, 0x0C,
    0xC0, 0x0D,
    0xC0, 0x0E,
    0xC0, 0x0F,
    0xC0, 0x10,
    0xC0, 0x11,
    0xC0, 0x7F,

    0xF0, 0x01, 0x02, 0x03, 0x04, 0xF7,

    0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xF7,

    0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0xF7,

    0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0xF7
};

#define BLE_MIDI_PACKETS_COUNT 19
byte packets[BLE_MIDI_PACKETS_COUNT][BLE_MAX_ATTR_DATA_LEN] = {
    {0x80, 0x80, 0x90, 0x01, 0x7F},
    {0x80, 0x80, 0x90, 0x02, 0x01, 0x02, 0x7F},
    {0x80, 0x80, 0x90, 0x03, 0x01, 0x03, 0x02, 0x03, 0x03, 0x03, 0x04, 0x03, 0x05, 0x03, 0x06, 0x03, 0x07, 0x03, 0x7F},
    {0x80, 0x80, 0x90, 0x03, 0x01, 0x80, 0x91, 0x03, 0x02, 0x80, 0x90, 0x03, 0x03, 0x03, 0x04, 0x03, 0x05, 0x03, 0x7F},
    {0x80, 0x80, 0xC0, 0x7F},
    {0x80, 0x80, 0xC0, 0x01, 0x80, 0xD0, 0x7F},
    {0x80, 0x80, 0xC0, 0x01, 0x02, 0x80, 0xD0, 0x01, 0x7F},
    {0x80, 0x80, 0xC0, 0x01, 0x02, 0x80, 0xD1, 0x01, 0x80, 0xD0, 0x7F},
    {0x80, 0x80, 0x90, 0x01, 0x01, 0x80, 0x91, 0x02, 0x02, 0x80, 0x93, 0x03, 0x03, 0x80, 0x94, 0x04, 0x04},
    {0x80, 0x80, 0x95, 0x05, 0x05, 0x80, 0x96, 0x01, 0x7F},
    {0x80, 0x80, 0x90, 0x01, 0x01, 0x80, 0xC0, 0x01, 0x02, 0x80, 0x91, 0x01, 0x7F},
    {0x80, 0x80, 0xC0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11},
    {0x80, 0x80, 0xC0, 0x7F},
    {0x80, 0x80, 0xF0, 0x01, 0x02, 0x03, 0x04, 0x80, 0xF7},
    {0x80, 0x80, 0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x80, 0xF7},
    {0x80, 0x80, 0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11},
    {0x80, 0x80, 0xF7},
    {0x80, 0x80, 0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11},
    {0x80, 0x12, 0x13, 0x80, 0xF7},
};

int packetsLength[BLE_MIDI_PACKETS_COUNT] = {
    5,
    7,
    19,
    19,
    4,
    7,
    9,
    11,
    17,
    9,
    13,
    20,
    4,
    9,
    20,
    20,
    3,
    20,
    5
};

int packetsCount = 0;
int bytesCount = 0;

class BLEService;

class BLEDevice {
public:
    BLEDevice();
    void begin();
    void setDeviceName(const char *name);
    void setLocalName(const char *name);
    void advertise();
    void addService(BLEService &service);
    void setAdvertisedServiceUuid(int uuid);
    void poll();
private:
    BLEService *service;
    int packetsPosition;
};

class BLECharacteristic {
public:
    BLECharacteristic(const char *uuid, int pars, int len);
    void setValue(byte *buffer, int len);
    void setEventHandler(int type, void (*callbackFunction)(BLEDevice, BLECharacteristic));
    int valueLength();
    byte* value();
    void fireEvent(BLEDevice &device, byte *buffer, int len);
private:
    void (*callbackFunction)(BLEDevice d, BLECharacteristic c);
    byte *buffer;
    int len;
};

class BLEService {
public:
    BLEService(const char *uuid);
    void addCharacteristic(BLECharacteristic &characteristic);
    int uuid();
    void fireEvent(BLEDevice &device, byte *buffer, int len);
private:
    BLECharacteristic *characteristic;
};

// -----------------------------------------------------------------------------

BLEDevice::BLEDevice() {
    this->service = NULL;
    this->packetsPosition = 0;
}

void BLEDevice::begin() {
}

void BLEDevice::setDeviceName(const char *name) {
}

void BLEDevice::setLocalName(const char *name) {
}

void BLEDevice::advertise() {
}

void BLEDevice::addService(BLEService &service) {
    this->service = &service;
}

void BLEDevice::setAdvertisedServiceUuid(int uuid) {
}

void BLEDevice::poll() {
    if (packetsPosition < BLE_MIDI_PACKETS_COUNT) {
        this->service->fireEvent(*this, packets[this->packetsPosition], packetsLength[this->packetsPosition]);
        this->packetsPosition++;
    }
}

// -----------------------------------------------------------------------------

BLECharacteristic::BLECharacteristic(const char *uuid, int pars, int len) {
    this->callbackFunction = NULL;
}

void BLECharacteristic::setValue(byte *buffer, int len) {
    printf("packet (%02i bytes):", len);
    for (int i = 0; i < len; i++) {
        printf(" %02X", buffer[i]);
    }
    printf("\n");
    if (memcmp(buffer, packets[packetsCount], len)) {
        printf("--------------------\n");
        printf("UNEXPECTED PACKET!!!\n");
        printf("--------------------\n");
        exit(1);
    }
    packetsCount++;
}

void BLECharacteristic::setEventHandler(int type, void (*callbackFunction)(BLEDevice, BLECharacteristic)) {
    this->callbackFunction = callbackFunction;
}

int BLECharacteristic::valueLength() {
    return this->len;
}

byte* BLECharacteristic::value() {
    return this->buffer;
}

void BLECharacteristic::fireEvent(BLEDevice &device, byte *buffer, int len) {
    this->buffer = buffer;
    this->len    = len;
    this->callbackFunction(device, *this);
}

// -----------------------------------------------------------------------------

BLEService::BLEService(const char *uuid) {
    this->characteristic = NULL;
}
void BLEService::addCharacteristic(BLECharacteristic &characteristic) {
    this->characteristic = &characteristic;
}
int BLEService::uuid() {
    return 0;
}
void BLEService::fireEvent(BLEDevice &device, byte *buffer, int len) {
    this->characteristic->fireEvent(device, buffer, len);
}

// -----------------------------------------------------------------------------

static unsigned long timeMicros = 0;

unsigned long micros() {
    return timeMicros;
}

class Serial {
public:
    unsigned int pointer;
    Serial() {
        pointer = 0;
    }
    void begin(int baudrate) {
    }
    int available() {
        int length = SERIAL_MIDI_BYTES_COUNT - pointer;
        if (!length) {
            timeMicros += MINIMUM_CONN_INTERVAL;
        }
        return length;
    }
    byte read() {
        byte b = testStream[pointer++];
        if (b == 0x7F || b == 0xF7) {
            timeMicros += MINIMUM_CONN_INTERVAL;
        }
        return b;
    }
    void write(byte b) {
        if (b != testStream[bytesCount] || bytesCount >= SERIAL_MIDI_BYTES_COUNT) {
            printf("--------------------------------------\n");
            printf("UNEXPECTED BYTE 0x%02X (expected 0x%02X)!!!\n", b, testStream[bytesCount]);
            printf("--------------------------------------\n");
            exit(1);
        }
        bytesCount++;
    }
};

// -----------------------------------------------------------------------------

BLEDevice BLE;
Serial Serial1;

#include "mourino.ino"

int main() {
    unsigned int count = 0;
    setup();
    while (packetsCount < BLE_MIDI_PACKETS_COUNT || bytesCount < SERIAL_MIDI_BYTES_COUNT) {
        loop();
        count++;
        if (count > SERIAL_MIDI_BYTES_COUNT + 4) {
            printf("--------------------------------------\n");
            if (packetsCount < BLE_MIDI_PACKETS_COUNT) {
                printf("PACKET NOT RECEIVED!!!\n");
            } else {
                printf("BYTE NOT SENT!!!\n");
            }
            printf("--------------------------------------\n");
            exit(1);
        }
    }
}
