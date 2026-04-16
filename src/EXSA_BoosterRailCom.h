#pragma once
#include <stdint.h>

/*
 * Module RailCom du Booster EXSA
 * ------------------------------
 * Rôle :
 *   - Échantillonner le courant voie pendant le cutout
 *   - Stocker les valeurs ADC dans un buffer
 *   - Décoder Channel 1 (adresse loco)
 *   - Décoder Channel 2 (CV, ACK, data) — structure prête
 *   - Remonter l’adresse détectée au Booster
 *
 * Fonctionnement :
 *   - onCutoutStart()  → début du cutout (PWM OFF)
 *   - update()         → échantillonnage ADC (pendant cutout)
 *   - onCutoutEnd()    → fin du cutout → décodage
 */

class EXSA_BoosterRailCom
{
public:
    static void begin();

    // Signaux envoyés par EXSA_Booster
    static void onCutoutStart();   // début cutout
    static void onCutoutEnd();     // fin cutout
    static void update();          // appelé toutes les 1 ms

    // Adresse détectée (0 = rien)
    static uint16_t getLastAddress();

private:
    // Échantillonnage ADC pendant le cutout
    static void sample();

    // Décodage complet du cutout
    static void decode();

    // Décodage d’un channel (1 ou 2)
    static uint8_t decodeChannel(const int *buf, int startIndex);

    // État interne
    static volatile bool _active;   // true = cutout en cours
    static volatile int  _index;    // position dans le buffer

    // Buffer d’échantillonnage
    static constexpr int BUF_SIZE = 128;
    static int _buffer[BUF_SIZE];

    // Dernière adresse détectée
    static uint16_t _lastAddress;
};
