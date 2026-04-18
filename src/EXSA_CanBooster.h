#pragma once
#include <stdint.h>
#include <ACAN_ESP32.h>

/*
 * ============================================================
 *  EXSA_CanBooster — Version Discovery 2026
 *  Réception DCC / Cutout / Télémétrie / RailCom via CAN
 * ============================================================
 */

class EXSA_CanBooster
{
public:
    // Initialisation du contrôleur CAN
    static void begin();

    // Traitement des trames entrantes (à appeler régulièrement)
    static void process();

    // Données partagées avec EXSA_Booster
    static volatile uint8_t  dccBuffer[8];
    static volatile uint8_t  dccLen;

    static volatile bool     cutoutActive;
    static volatile uint16_t railcomAddress;

    static volatile uint16_t voieCourant_mA;
    static volatile uint16_t voieTension_mV;
    static volatile uint8_t  boosterState;

private:
    // Dispatch des trames CAN
    static void handleFrame(const CANMessage &msg);

    // Handlers individuels
    static void onDccFrame(const CANMessage &msg);
    static void onCutoutFrame(const CANMessage &msg);
    static void onTelemetryFrame(const CANMessage &msg);
    static void onRailcomFrame(const CANMessage &msg);
};
