#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_BoosterHw — Version patchée Discovery 2026
 *  Gestion DRV8801 + PWM DCC + ADC + RailCom
 * ============================================================
 */

class EXSA_BoosterHw
{
public:
    // Initialisation hardware complète
    static void begin();

    // DCC physique (bitstream reçu via CAN)
    static void applyDcc(const uint8_t *data, uint8_t len);

    // Cutout RailCom (activation / désactivation)
    static void enableCutout();
    static void disableCutout();

    // DRV8801 (pont en H)
    static void enableOutput();
    static void disableOutput();
    static bool  isFaultActive();

    // Télémétrie voie
    static uint16_t readCurrent_mA();
    static uint16_t readVoltage_mV();

    // RailCom — lecture ADC haute fréquence
    static int16_t readRailcomAdcRaw();

private:
    // Sous-systèmes internes
    static void setupPwmDcc();
    static void setupDrv8801();
    static void setupAdc();
};
