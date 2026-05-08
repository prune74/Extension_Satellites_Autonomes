/*
 * EXSA_Booster.cpp
 * ------------------------------------------------------------
 * Booster local Discovery 2026 — cœur logique voie DCC.
 *
 * Gère :
 *   - application du bit DCC reçu via CAN
 *   - cutout RailCom (fenêtre de mesure)
 *   - télémétrie courant / tension (CAN 0x102)
 *   - publication des adresses RailCom (CAN 0x103)
 *   - sécurité voie (locale + globale multi-boosters)
 */

#include "EXSA_Booster.h"
#include "EXSA_CanBooster.h"
#include "EXSA_BoosterHw.h"
#include "EXSA_BoosterRailCom.h"
#include "EXSA_Config.h"

#include <Arduino.h>

/* ============================================================
   Timers internes (ms)
   ============================================================ */
uint32_t EXSA_Booster::_lastTelemetryTime = 0;
uint32_t EXSA_Booster::_lastRailcomTime   = 0;

/* ============================================================
   État local du booster
   ------------------------------------------------------------
   0 = OK
   1 = Court-circuit
   2 = Surchauffe (réservé)
   3 = Voie OFF / tension anormale
   ============================================================ */
static uint8_t s_localBoosterState = 0;

/* ============================================================
   begin() — Initialisation hardware + RailCom
   ============================================================ */
void EXSA_Booster::begin()
{
    EXSA_BoosterHw::begin();
    EXSA_BoosterRailCom::begin();
}

/* ============================================================
   update() — Boucle principale (appelée toutes les 1 ms)
   ============================================================ */
void EXSA_Booster::update()
{
    updateCutout();                 // cutout RailCom
    applyDccFrame();                // DCC physique
    updateTelemetry();              // courant / tension + état
    EXSA_BoosterRailCom::update();  // hook éventuel
    updateRailcom();                // adresse RailCom
    checkSafety();                  // sécurité voie
}

/* ============================================================
   applyDccFrame() — Application du bit DCC reçu via CAN
   ============================================================ */
void EXSA_Booster::applyDccFrame()
{
    if (EXSA_CanBooster::dccLen == 0)
        return;

    EXSA_BoosterHw::applyDcc(
        (const uint8_t*)EXSA_CanBooster::dccBuffer,
        EXSA_CanBooster::dccLen
    );
}

/* ============================================================
   updateCutout() — Gestion de la fenêtre RailCom
   ============================================================ */
void EXSA_Booster::updateCutout()
{
    if (EXSA_CanBooster::cutoutActive)
    {
        EXSA_BoosterHw::enableCutout();        // EN = 0 (PWM OFF)
        EXSA_BoosterRailCom::onCutoutStart();
    }
    else
    {
        EXSA_BoosterHw::disableCutout();       // EN = 255 (PWM ON)
        EXSA_BoosterRailCom::onCutoutEnd();
    }
}

/* ============================================================
   updateTelemetry() — Mesure + envoi CAN 0x102
   ------------------------------------------------------------
   Cadence : toutes les 20 ms.
   data[0] = courant (mA / 10, saturé à 255)
   data[1] = tension (mV / 100, saturé à 255)
   data[2] = état booster (s_localBoosterState)
   ============================================================ */
void EXSA_Booster::updateTelemetry()
{
    const uint32_t now = millis();

    if (now - _lastTelemetryTime < 20)
        return;

    // Pas de mesure pendant le cutout
    if (EXSA_CanBooster::cutoutActive)
        return;

    _lastTelemetryTime = now;

    uint16_t courant = EXSA_BoosterHw::readCurrent_mA();
    uint16_t tension = EXSA_BoosterHw::readVoltage_mV();

    // --- Calcul de l’état local en fonction des seuils config ---
    if (courant > EXSA_BOOSTER_MAX_COURANT_mA)
    {
        s_localBoosterState = 1; // Court-circuit
    }
    else if (tension < EXSA_BOOSTER_MIN_TENSION_mV)
    {
        s_localBoosterState = 3; // Voie OFF / alim faible
    }
    else
    {
        s_localBoosterState = 0; // OK
    }

    // Propagation vers EXSA_CanBooster (pour les autres modules)
    EXSA_CanBooster::boosterState = s_localBoosterState;

    // --- ENVOI CAN TÉLÉMÉTRIE (0x102) ---
    uint8_t courant8 = min<uint16_t>(courant / 10, 255);
    uint8_t tension8 = min<uint16_t>(tension / 100, 255);

    CANMessage msg;
    msg.id  = 0x102;
    msg.len = 3;
    msg.data[0] = courant8;
    msg.data[1] = tension8;
    msg.data[2] = s_localBoosterState;

    ACAN_ESP32::can.tryToSend(msg);
}

/* ============================================================
   updateRailcom() — Publication CAN 0x103
   ============================================================ */
void EXSA_Booster::updateRailcom()
{
    const uint32_t now = millis();

    if (now - _lastRailcomTime < 10)
        return;

    _lastRailcomTime = now;

    uint16_t addr = EXSA_BoosterRailCom::getLastAddress();

    if (addr != 0)
    {
        CANMessage msg;
        msg.id  = 0x103;
        msg.len = 2;
        msg.data[0] = addr & 0xFF;
        msg.data[1] = (addr >> 8) & 0xFF;

        ACAN_ESP32::can.tryToSend(msg);

        EXSA_BoosterRailCom::clearLastAddress();
    }
}

/* ============================================================
   checkSafety() — Sécurité voie Discovery 2026
   ------------------------------------------------------------
   Règles :
     1) FAULT DRV8874 (hardware, nFAULT = LOW)
     2) Court-circuit local (courant > EXSA_BOOSTER_MAX_COURANT_mA)
     3) Tension anormale (tension < EXSA_BOOSTER_MIN_TENSION_mV)
     4) Défauts des autres boosters (globalFault / globalOverheat / globalOff)
     5) Inversion de phase (phaseMismatch)
     6) Cutout global (globalCutout)
   ============================================================ */
void EXSA_Booster::checkSafety()
{
    // 1) Sécurité hardware DRV8874 (nFAULT = LOW)
    if (EXSA_BoosterHw::isFaultActive())
    {
        EXSA_BoosterHw::disableOutput();
        return;
    }

    // Lecture locale (pour sécurité temps réel, même si déjà lu en télémétrie)
    uint16_t courant = EXSA_BoosterHw::readCurrent_mA();
    uint16_t tension = EXSA_BoosterHw::readVoltage_mV();

    // 2) Court-circuit local
    if (courant > EXSA_BOOSTER_MAX_COURANT_mA)
    {
        EXSA_BoosterHw::disableOutput();
        s_localBoosterState = 1;
        EXSA_CanBooster::boosterState = 1;
        return;
    }

    // 3) Tension anormale / voie OFF
    if (tension < EXSA_BOOSTER_MIN_TENSION_mV)
    {
        EXSA_BoosterHw::disableOutput();
        s_localBoosterState = 3;
        EXSA_CanBooster::boosterState = 3;
        return;
    }

    // 4) Défauts des autres boosters (protection globale optionnelle)
    if (EXSA_BOOSTER_ENABLE_GLOBAL_PROTECTION)
    {
        if (EXSA_CanBooster::globalFault ||
            EXSA_CanBooster::globalOverheat ||
            EXSA_CanBooster::globalOff)
        {
            EXSA_BoosterHw::disableOutput();
            return;
        }
    }

    // 5) Inversion de phase détectée
    if (EXSA_CanBooster::phaseMismatch)
    {
        EXSA_BoosterHw::disableOutput();
        return;
    }

    // 6) Cutout global (optionnel)
    if (EXSA_BOOSTER_ENABLE_GLOBAL_CUTOUT &&
        EXSA_CanBooster::globalCutout)
    {
        EXSA_BoosterHw::disableOutput();
        return;
    }

    // Si aucune erreur → voie active
    EXSA_BoosterHw::enableOutput();
}
