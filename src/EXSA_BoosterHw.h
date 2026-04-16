#pragma once
#include <stdint.h>

class EXSA_BoosterHw
{
public:
    // Initialisation hardware (DRV8801, PWM, ADC…)
    static void begin();

    // --- DCC ---
    static void applyDcc(const uint8_t *data, uint8_t len);

    // --- Cutout RailCom ---
    static void enableCutout();
    static void disableCutout();

    // --- DRV8801 ---
    static void enableOutput();
    static void disableOutput();

    // --- Télémétrie voie ---
    static uint16_t readCurrent_mA();
    static uint16_t readVoltage_mV();

private:
    // Fonctions internes
    static void setupPwmDcc();
    static void setupDrv8801();
    static void setupAdc();
};
