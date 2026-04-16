#include "EXSA_CanBooster.h"

// Variables partagées
volatile uint8_t  EXSA_CanBooster::dccBuffer[8] = {0};
volatile uint8_t  EXSA_CanBooster::dccLen       = 0;

volatile bool     EXSA_CanBooster::cutoutActive = false;
volatile uint16_t EXSA_CanBooster::railcomAddress = 0;

volatile uint16_t EXSA_CanBooster::voieCourant_mA = 0;
volatile uint16_t EXSA_CanBooster::voieTension_mV = 0;
volatile uint8_t  EXSA_CanBooster::boosterState   = 0;

// ------------------------------------------------------------
// Initialisation du CAN Booster
// ------------------------------------------------------------
void EXSA_CanBooster::begin()
{
    ACAN_ESP32_Settings settings(500000); // 500 kbps typique
    settings.mRxPin = GPIO_NUM_4;         // À ajuster
    settings.mTxPin = GPIO_NUM_5;         // À ajuster

    const uint32_t err = ACAN_ESP32::can.begin(settings);
    if (err != 0) {
        // TODO : gestion erreur
    }
}

// ------------------------------------------------------------
// Tâche CAN Booster : lecture des trames
// ------------------------------------------------------------
void EXSA_CanBooster::process()
{
    CANMessage msg;
    while (ACAN_ESP32::can.receive(msg)) {
        handleFrame(msg);
    }
}

// ------------------------------------------------------------
// Dispatch selon l’ID
// ------------------------------------------------------------
void EXSA_CanBooster::handleFrame(const CANMessage &msg)
{
    switch (msg.id)
    {
        case 0x100: onDccFrame(msg);      break;
        case 0x101: onCutoutFrame(msg);   break;
        case 0x102: onTelemetryFrame(msg);break;
        case 0x103: onRailcomFrame(msg);  break;
        default:
            // Trame inconnue → ignorée
            break;
    }
}

// ------------------------------------------------------------
// 0x100 : trame DCC encapsulée
// ------------------------------------------------------------
void EXSA_CanBooster::onDccFrame(const CANMessage &msg)
{
    dccLen = msg.len;
    for (uint8_t i = 0; i < msg.len; i++)
        dccBuffer[i] = msg.data[i];
}

// ------------------------------------------------------------
// 0x101 : état cutout RailCom
// ------------------------------------------------------------
void EXSA_CanBooster::onCutoutFrame(const CANMessage &msg)
{
    cutoutActive = (msg.data[0] != 0);
}

// ------------------------------------------------------------
// 0x102 : télémétrie voie
// ------------------------------------------------------------
void EXSA_CanBooster::onTelemetryFrame(const CANMessage &msg)
{
    voieCourant_mA = msg.data[0] * 10;   // ex : 123 → 1230 mA
    voieTension_mV = msg.data[1] * 100;  // ex : 120 → 12.0 V
    boosterState   = msg.data[2];
}

// ------------------------------------------------------------
// 0x103 : adresse RailCom détectée
// ------------------------------------------------------------
void EXSA_CanBooster::onRailcomFrame(const CANMessage &msg)
{
    railcomAddress = (uint16_t(msg.data[1]) << 8) | msg.data[0];
}
