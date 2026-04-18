#pragma once
#include <stdint.h>

class EXSA_BoosterHw
{
public:
    static void begin();

    // DCC
    static void applyDcc(const uint8_t *data, uint8_t len);

    // Cutout RailCom
    static void enableCutout();
    static void disableCutout();

    // DRV8801
    static void enableOutput();
    static void disableOutput();
    static bool  isFaultActive();

    // Télémétrie voie
    static uint16_t readCurrent_mA();
    static uint16_t readVoltage_mV();

    // RailCom (ADC brut haute fréquence)
    static int16_t readRailcomAdcRaw();

private:
    static void setupPwmDcc();
    static void setupDrv8801();
    static void setupAdc();
};
