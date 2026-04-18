#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_Booster — Booster local Discovery 2026
 *  Version patchée : RailCom long + CAN + cohérence LaBox
 * ============================================================
 */

class EXSA_Booster
{
public:
    // Initialisation hardware + RailCom
    static void begin();

    // Appelée toutes les 1 ms depuis la loop principale
    static void update();

private:
    // Sous-fonctions internes
    static void applyDccFrame();
    static void updateCutout();
    static void updateTelemetry();
    static void updateRailcom();
    static void checkSafety();

    // Timers internes (ms)
    static uint32_t _lastTelemetryTime;
    static uint32_t _lastRailcomTime;
};
