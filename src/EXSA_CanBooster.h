#pragma once
#include <stdint.h>
#include <ACAN_ESP32.h>

class EXSA_CanBooster
{
public:
    static void begin();
    static void process();

    static volatile uint8_t  dccBuffer[8];
    static volatile uint8_t  dccLen;

    static volatile bool     cutoutActive;
    static volatile uint16_t railcomAddress;
    static volatile uint16_t voieCourant_mA;
    static volatile uint16_t voieTension_mV;
    static volatile uint8_t  boosterState;

private:
    static void handleFrame(const CANMessage &msg);

    static void onDccFrame(const CANMessage &msg);
    static void onCutoutFrame(const CANMessage &msg);
    static void onTelemetryFrame(const CANMessage &msg);
    static void onRailcomFrame(const CANMessage &msg);
};
