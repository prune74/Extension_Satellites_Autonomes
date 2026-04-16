#pragma once
#include <stdint.h>
#include <ACAN_ESP32.h>

class EXSA_CanBooster
{
public:
    // Initialisation du CAN Booster
    static void begin();

    // Appelé régulièrement par la tâche BoosterCAN_Task
    static void process();

    // Dernières données reçues (thread-safe si besoin)
    static volatile uint8_t  dccBuffer[8];
    static volatile uint8_t  dccLen;

    static volatile bool     cutoutActive;
    static volatile uint16_t railcomAddress;
    static volatile uint16_t voieCourant_mA;
    static volatile uint16_t voieTension_mV;
    static volatile uint8_t  boosterState;

private:
    // Décodage d’une trame CAN reçue
    static void handleFrame(const CANMessage &msg);

    // Décodage des différents types de trames
    static void onDccFrame(const CANMessage &msg);
    static void onCutoutFrame(const CANMessage &msg);
    static void onTelemetryFrame(const CANMessage &msg);
    static void onRailcomFrame(const CANMessage &msg);
};
