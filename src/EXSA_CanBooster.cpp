#include "EXSA_CanBooster.h"

// Variables partagées
volatile uint8_t  EXSA_CanBooster::dccBuffer[8] = {0};
volatile uint8_t  EXSA_CanBooster::dccLen       = 0;

volatile bool     EXSA_CanBooster::cutoutActive   = false;
volatile uint16_t EXSA_CanBooster::railcomAddress = 0;

volatile uint16_t EXSA_CanBooster::voieCourant_mA = 0;
volatile uint16_t EXSA_CanBooster::voieTension_mV = 0;
volatile uint8_t  EXSA_CanBooster::boosterState   = 0;

/*
 * ============================================================
 *  Initialisation CAN — Discovery 2026
 * ============================================================
 */

void EXSA_CanBooster::begin()
{
    ACAN_ESP32_Settings settings(500000); // 500 kbps
    settings.mRxPin = GPIO_NUM_4;         // à ajuster selon carte
    settings.mTxPin = GPIO_NUM_5;

    const uint32_t err = ACAN_ESP32::can.begin(settings);
    if (err != 0) {
        // TODO : gestion erreur (LED, log, etc.)
    }
}

/*
 * ============================================================
 *  Boucle de traitement CAN
 * ============================================================
 */

void EXSA_CanBooster::process()
{
    CANMessage msg;
    while (ACAN_ESP32::can.receive(msg)) {
        handleFrame(msg);
    }
}

/*
 * ============================================================
 *  Dispatch des trames CAN
 * ============================================================
 */

void EXSA_CanBooster::handleFrame(const CANMessage &msg)
{
    switch (msg.id)
    {
        case 0x100: onDccFrame(msg);       break;
        case 0x101: onCutoutFrame(msg);    break;
        case 0x102: onTelemetryFrame(msg); break;
        case 0x103: onRailcomFrame(msg);   break;
        default:
            break;
    }
}

/*
 * ============================================================
 *  Réception DCC (0x100)
 * ============================================================
 */

void EXSA_CanBooster::onDccFrame(const CANMessage &msg)
{
    dccLen = msg.len;
    for (uint8_t i = 0; i < msg.len; i++)
        dccBuffer[i] = msg.data[i];
}

/*
 * ============================================================
 *  Réception Cutout (0x101)
 * ============================================================
 */

void EXSA_CanBooster::onCutoutFrame(const CANMessage &msg)
{
    cutoutActive = (msg.data[0] != 0);
}

/*
 * ============================================================
 *  Réception Télémétrie (0x102)
 *  (utile si un autre booster envoie ses infos)
 * ============================================================
 */

void EXSA_CanBooster::onTelemetryFrame(const CANMessage &msg)
{
    voieCourant_mA = msg.data[0] * 10;   // ex : 123 → 1230 mA
    voieTension_mV = msg.data[1] * 100;  // ex : 120 → 12.0 V
    boosterState   = msg.data[2];
}

/*
 * ============================================================
 *  Réception RailCom (0x103)
 * ============================================================
 */

void EXSA_CanBooster::onRailcomFrame(const CANMessage &msg)
{
    railcomAddress = (uint16_t(msg.data[1]) << 8) | msg.data[0];
}
