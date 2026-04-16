#include "EXSA_Booster.h"
#include "EXSA_CanBooster.h"
#include "EXSA_BoosterHw.h"
#include "EXSA_BoosterRailCom.h"

#include <Arduino.h>

// Timers internes
uint32_t EXSA_Booster::_lastDccTime       = 0;
uint32_t EXSA_Booster::_lastTelemetryTime = 0;
uint32_t EXSA_Booster::_lastRailcomTime   = 0;

// ------------------------------------------------------------
// Initialisation du booster
// ------------------------------------------------------------
void EXSA_Booster::begin()
{
    EXSA_BoosterHw::begin();          // init DRV8801, PWM, ADC
    EXSA_BoosterRailCom::begin();     // init RailCom              ← AJOUT
}

// ------------------------------------------------------------
// Boucle temps réel (appelée toutes les 1 ms)
// ------------------------------------------------------------
void EXSA_Booster::update()
{
    applyDccFrame();
    updateCutout();
    updateTelemetry();

    EXSA_BoosterRailCom::update();    // RailCom pendant cutout    ← AJOUT

    updateRailcom();
    checkSafety();
}

// ------------------------------------------------------------
// Application du DCC reçu via CAN Booster
// ------------------------------------------------------------
void EXSA_Booster::applyDccFrame()
{
    const uint32_t now = millis();

    if (now - _lastDccTime < 1)
        return;

    _lastDccTime = now;

    if (EXSA_CanBooster::dccLen == 0)
        return;

    EXSA_BoosterHw::applyDcc(
        (const uint8_t*)EXSA_CanBooster::dccBuffer,   // ← FIX
        EXSA_CanBooster::dccLen
    );
}

// ------------------------------------------------------------
// Gestion du cutout RailCom
// ------------------------------------------------------------
void EXSA_Booster::updateCutout()
{
    if (EXSA_CanBooster::cutoutActive)
    {
        EXSA_BoosterHw::enableCutout();
        EXSA_BoosterRailCom::onCutoutStart();   // ← AJOUT
    }
    else
    {
        EXSA_BoosterHw::disableCutout();
        EXSA_BoosterRailCom::onCutoutEnd();     // ← AJOUT
    }
}

// ------------------------------------------------------------
// Mise à jour télémétrie voie (courant, tension)
// ------------------------------------------------------------
void EXSA_Booster::updateTelemetry()
{
    const uint32_t now = millis();

    if (now - _lastTelemetryTime < 20)
        return;

    _lastTelemetryTime = now;

    uint16_t courant = EXSA_BoosterHw::readCurrent_mA();
    uint16_t tension = EXSA_BoosterHw::readVoltage_mV();

    // TODO : envoyer télémétrie au CAN Booster
    (void)courant;
    (void)tension;
}

// ------------------------------------------------------------
// Mise à jour RailCom (adresse détectée)
// ------------------------------------------------------------
void EXSA_Booster::updateRailcom()
{
    const uint32_t now = millis();

    if (now - _lastRailcomTime < 10)
        return;

    _lastRailcomTime = now;

    // Adresse détectée sur la voie
    uint16_t addr = EXSA_BoosterRailCom::getLastAddress();   // ← MODIFIÉ

    if (addr != 0)
    {
        // TODO : remonter via CAN Booster
        // EXSA_CanBooster::sendRailcomAddress(addr);
    }
}

// ------------------------------------------------------------
// Sécurité voie (court-circuit, surchauffe…)
// ------------------------------------------------------------
void EXSA_Booster::checkSafety()
{
    uint8_t state = EXSA_CanBooster::boosterState;

    switch (state)
    {
        case 0: // OK
            EXSA_BoosterHw::enableOutput();
            break;

        case 1: // Court-circuit
        case 2: // Surchauffe
        case 3: // Voie OFF
            EXSA_BoosterHw::disableOutput();
            break;

        default:
            break;
    }
}
