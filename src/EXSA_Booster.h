#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_Booster — Booster local Discovery 2026
 * ============================================================
 */

class EXSA_Booster
{
public:
    static void begin();
    static void update();   // appelée toutes les 1 ms

private:
    static void applyDccFrame();
    static void updateCutout();
    static void updateTelemetry();
    static void updateRailcom();
    static void checkSafety();

    static uint32_t _lastTelemetryTime;
    static uint32_t _lastRailcomTime;
};
