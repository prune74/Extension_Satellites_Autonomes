/*
 * EXSA_CanBooster.cpp
 * ------------------------------------------------------------
 * Module CAN Booster Discovery 2026.
 *
 * Rôle :
 *   - Réception des trames CAN envoyées par LaBox :
 *        • 0x100 : bit DCC + phase
 *        • 0x101 : cutout local + cutout global
 *        • 0x102 : télémétrie d’un autre booster
 *        • 0x103 : adresse RailCom d’un autre booster
 *
 *   - Mise à disposition des données pour EXSA_Booster :
 *        • dccBuffer / dccLen
 *        • cutoutActive
 *        • railcomAddress
 *        • voieCourant_mA / voieTension_mV
 *        • boosterState (état local ou reçu)
 *
 *   - Gestion des sécurités globales multi-boosters :
 *        • globalFault
 *        • globalOverheat
 *        • globalOff
 *        • phaseMismatch
 *        • globalCutout
 *
 * Ce module ne génère rien : il ne fait que recevoir.
 */

#include "EXSA_CanBooster.h"
#include "EXSA_Pins.h"

// ============================================================================
// Variables partagées (DCC, cutout, RailCom, télémétrie)
// ============================================================================
volatile uint8_t  EXSA_CanBooster::dccBuffer[8] = {0};
volatile uint8_t  EXSA_CanBooster::dccLen       = 0;

volatile bool     EXSA_CanBooster::cutoutActive = false;
volatile bool     EXSA_CanBooster::globalCutout = false;

volatile uint16_t EXSA_CanBooster::railcomAddress = 0;

volatile uint16_t EXSA_CanBooster::voieCourant_mA = 0;
volatile uint16_t EXSA_CanBooster::voieTension_mV = 0;
volatile uint8_t  EXSA_CanBooster::boosterState   = 0;

// ============================================================================
// Variables globales de sécurité multi-boosters
// ============================================================================
volatile bool EXSA_CanBooster::globalFault     = false;
volatile bool EXSA_CanBooster::globalOverheat  = false;
volatile bool EXSA_CanBooster::globalOff       = false;
volatile bool EXSA_CanBooster::phaseMismatch   = false;

// Phase attendue (mise à jour par LaBox)
static volatile uint8_t s_expectedPhase = 0;

// ============================================================================
// Initialisation CAN — Version compatible ACAN_ESP32 simple
// ============================================================================
void EXSA_CanBooster::begin()
{
    ACAN_ESP32_Settings settings(500000); // 500 kbps

    settings.mRxPin = GPIO_NUM_4;
    settings.mTxPin = GPIO_NUM_5;

    const uint32_t err = ACAN_ESP32::can.begin(settings);
    if (err != 0) {

        // Sécurité Discovery 2026
        globalFault = true;
        globalOff   = true;

        // LED rouge ON
        pinMode(EXSA_LED_ERROR_PIN, OUTPUT);
        digitalWrite(EXSA_LED_ERROR_PIN, HIGH);

        return;
    }
}

// ============================================================================
// Boucle de traitement CAN
// ============================================================================
void EXSA_CanBooster::process()
{
    CANMessage msg;
    while (ACAN_ESP32::can.receive(msg)) {
        handleFrame(msg);
    }
}

// ============================================================================
// Dispatch des trames CAN
// ============================================================================
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

// ============================================================================
// Réception DCC (0x100)
// data[0] = bit DCC
// data[1] = phase (0 ou 1)
// ============================================================================
void EXSA_CanBooster::onDccFrame(const CANMessage &msg)
{
    dccLen = msg.len;
    for (uint8_t i = 0; i < msg.len; i++)
        dccBuffer[i] = msg.data[i];

    uint8_t receivedPhase = msg.data[1];

    // Détection inversion de phase
    phaseMismatch = (receivedPhase != s_expectedPhase);
    s_expectedPhase = receivedPhase;
}

// ============================================================================
// Réception Cutout (0x101)
// data[0] = cutout local
// data[1] = cutout global
// ============================================================================
void EXSA_CanBooster::onCutoutFrame(const CANMessage &msg)
{
    cutoutActive = (msg.data[0] != 0);
    globalCutout = (msg.data[1] != 0);
}

// ============================================================================
// Réception Télémétrie (0x102)
// data[0] = courant (x10 mA)
// data[1] = tension (x100 mV)
// data[2] = état booster émetteur
// ============================================================================
void EXSA_CanBooster::onTelemetryFrame(const CANMessage &msg)
{
    voieCourant_mA = msg.data[0] * 10;
    voieTension_mV = msg.data[1] * 100;

    uint8_t state = msg.data[2];

    globalFault     = (state == 1);
    globalOverheat  = (state == 2);
    globalOff       = (state == 3);
}

// ============================================================================
// Réception RailCom (0x103)
// ============================================================================
void EXSA_CanBooster::onRailcomFrame(const CANMessage &msg)
{
    railcomAddress = (uint16_t(msg.data[1]) << 8) | msg.data[0];
}
