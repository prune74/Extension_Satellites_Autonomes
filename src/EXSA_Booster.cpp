#include "EXSA_Booster.h"
#include "EXSA_CanBooster.h"
#include "EXSA_BoosterHw.h"
#include "EXSA_BoosterRailCom.h"

#include <Arduino.h>

/*
 * ============================================================
 *  EXSA_Booster — Booster local Discovery 2026 (optimisé)
 * ============================================================
 */

uint32_t EXSA_Booster::_lastTelemetryTime = 0;
uint32_t EXSA_Booster::_lastRailcomTime   = 0;

void EXSA_Booster::begin()
{
    EXSA_BoosterHw::begin();
    EXSA_BoosterRailCom::begin();
}

void EXSA_Booster::update()
{
    updateCutout();                 // cutout RailCom
    applyDccFrame();                // DCC physique
    updateTelemetry();              // courant / tension
    EXSA_BoosterRailCom::update();  // hook éventuel
    updateRailcom();                // adresse RailCom
    checkSafety();                  // sécurité voie
}

void EXSA_Booster::applyDccFrame()
{
    if (EXSA_CanBooster::dccLen == 0)
        return;

    EXSA_BoosterHw::applyDcc(
        (const uint8_t*)EXSA_CanBooster::dccBuffer,
        EXSA_CanBooster::dccLen
    );
}

void EXSA_Booster::updateCutout()
{
    if (EXSA_CanBooster::cutoutActive)
    {
        EXSA_BoosterHw::enableCutout();
        EXSA_BoosterRailCom::onCutoutStart();
    }
    else
    {
        EXSA_BoosterHw::disableCutout();
        EXSA_BoosterRailCom::onCutoutEnd();
    }
}

void EXSA_Booster::updateTelemetry()
{
    const uint32_t now = millis();

    if (now - _lastTelemetryTime < 20)
        return;

    if (EXSA_CanBooster::cutoutActive)
        return;

    _lastTelemetryTime = now;

    uint16_t courant = EXSA_BoosterHw::readCurrent_mA();
    uint16_t tension = EXSA_BoosterHw::readVoltage_mV();

    // --- ENVOI CAN TÉLÉMÉTRIE (0x102) ---
    uint8_t courant8 = min<uint16_t>(courant / 10, 255);
    uint8_t tension8 = min<uint16_t>(tension / 100, 255);

    CANMessage msg;
    msg.id  = 0x102;
    msg.len = 3;
    msg.data[0] = courant8;
    msg.data[1] = tension8;
    msg.data[2] = EXSA_CanBooster::boosterState;

    ACAN_ESP32::can.tryToSend(msg);
}

void EXSA_Booster::updateRailcom()
{
    const uint32_t now = millis();

    if (now - _lastRailcomTime < 10)
        return;

    _lastRailcomTime = now;

    uint16_t addr = EXSA_BoosterRailCom::getLastAddress();

    if (addr != 0)
    {
        // --- ENVOI CAN RAILCOM (0x103) ---
        CANMessage msg;
        msg.id  = 0x103;
        msg.len = 2;
        msg.data[0] = addr & 0xFF;
        msg.data[1] = (addr >> 8) & 0xFF;

        ACAN_ESP32::can.tryToSend(msg);

        EXSA_BoosterRailCom::clearLastAddress();
    }
}

void EXSA_Booster::checkSafety()
{
    // Sécurité DRV8801 hardware
    if (EXSA_BoosterHw::isFaultActive())
    {
        EXSA_BoosterHw::disableOutput();
        return;
    }

    // Sécurité CAN Booster
    switch (EXSA_CanBooster::boosterState)
    {
        case 0: // OK
            EXSA_BoosterHw::enableOutput();
            break;

        case 1: // Court-circuit
        case 2: // Surchauffe
        case 3: // Voie OFF
            EXSA_BoosterHw::disableOutput();
            break;
    }
}
