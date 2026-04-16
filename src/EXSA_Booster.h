#pragma once
#include <stdint.h>

class EXSA_Booster
{
public:
    // Initialisation du booster (appelée dans BoosterCore_Task)
    static void begin();

    // Mise à jour temps réel (appelée toutes les 1 ms)
    static void update();

private:
    // --- Gestion DCC ---
    static void applyDccFrame();

    // --- Gestion cutout RailCom ---
    static void updateCutout();

    // --- Gestion télémétrie voie ---
    static void updateTelemetry();

    // --- Gestion RailCom (adresse détectée) ---
    static void updateRailcom();

    // --- Sécurité voie ---
    static void checkSafety();

    // Variables internes
    static uint32_t _lastDccTime;
    static uint32_t _lastTelemetryTime;
    static uint32_t _lastRailcomTime;
};
