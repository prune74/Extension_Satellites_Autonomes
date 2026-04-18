#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_BoosterRailCom — Version Discovery 2026
 *  Décodage Locoduino : adresses longues + filtrage
 * ============================================================
 */

class EXSA_BoosterRailCom
{
public:
    // Initialisation du module RailCom
    static void begin();

    // Appelé au début du cutout
    static void onCutoutStart();

    // Appelé à la fin du cutout (déclenche le décodage)
    static void onCutoutEnd();

    // Hook optionnel (appelé toutes les 1 ms)
    static void update();

    // Adresse décodée (0 si rien)
    static uint16_t getLastAddress();
    static void     clearLastAddress();

    // Membres accessibles par l’ISR
    static volatile bool   _active;
    static volatile int    _index;
    static const int       BUF_SIZE = 128;
    static int16_t         _buffer[BUF_SIZE];

private:
    // Dernière adresse stable (filtrée)
    static uint16_t _lastAddress;

    // Décodage complet RailCom (Locoduino)
    static void decode();

    // Décodage d’un canal (8 bits)
    static uint8_t decodeChannel(const int16_t *buf,
                                 int startIndex,
                                 int length);
};
