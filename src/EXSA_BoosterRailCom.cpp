#include "EXSA_BoosterRailCom.h"
#include "EXSA_BoosterHw.h"
#include <Arduino.h>

// Indique si on est dans un cutout
volatile bool EXSA_BoosterRailCom::_active = false;

// Position d’écriture dans le buffer
volatile int EXSA_BoosterRailCom::_index = 0;

// Buffer d’échantillonnage ADC
int EXSA_BoosterRailCom::_buffer[BUF_SIZE];

// Dernière adresse détectée
uint16_t EXSA_BoosterRailCom::_lastAddress = 0;

/* ============================================================
   Initialisation
   ============================================================ */
void EXSA_BoosterRailCom::begin()
{
    _active = false;
    _index = 0;
    _lastAddress = 0;
}

/* ============================================================
   Début du cutout
   ============================================================ */
void EXSA_BoosterRailCom::onCutoutStart()
{
    _active = true;   // on commence à échantillonner
    _index = 0;       // on repart au début du buffer
}

/* ============================================================
   Fin du cutout → décodage
   ============================================================ */
void EXSA_BoosterRailCom::onCutoutEnd()
{
    _active = false;  // on arrête l’échantillonnage
    decode();         // on analyse le buffer
}

/* ============================================================
   Appelé toutes les 1 ms par EXSA_Booster
   ============================================================ */
void EXSA_BoosterRailCom::update()
{
    if (_active)
        sample();     // on lit l’ADC tant que le cutout est actif
}

/* ============================================================
   Échantillonnage ADC pendant le cutout
   ============================================================ */
void EXSA_BoosterRailCom::sample()
{
    // Sécurité : éviter de dépasser le buffer
    if (_index >= BUF_SIZE)
        return;

    // Lecture du courant voie (ADC)
    _buffer[_index++] = EXSA_BoosterHw::readCurrent_mA();
}

/* ============================================================
   Décodage complet du cutout
   ============================================================ */
void EXSA_BoosterRailCom::decode()
{
    // Trop peu d’échantillons → cutout invalide
    if (_index < 40)
        return;

    /*
     * Découpage du cutout :
     *  - Channel 1 ≈ 25 % du cutout
     *  - Channel 2 ≈ 75 % du cutout
     */
    int mid1 = _index * 0.25;
    int mid2 = _index * 0.75;

    // Décodage des deux channels
    uint8_t ch1 = decodeChannel(_buffer, mid1);
    uint8_t ch2 = decodeChannel(_buffer, mid2);

    // Channel 1 = adresse loco
    if (ch1 != 0)
        _lastAddress = ch1;

    // Channel 2 = CV / ACK / data (à implémenter)
    (void)ch2;
}

/* ============================================================
   Décodage d’un channel RailCom
   ============================================================ */
uint8_t EXSA_BoosterRailCom::decodeChannel(const int *buf, int start)
{
    /*
     * 1) Calcul d’un seuil dynamique
     *    → permet de s’adapter au bruit et aux variations
     */
    int threshold = 0;
    for (int i = 0; i < BUF_SIZE; i++)
        threshold += buf[i];
    threshold /= BUF_SIZE;

    /*
     * 2) Lecture des 8 bits de data
     *    Chaque bit dure environ 20–25 µs
     *    On prend un échantillon au milieu de chaque bit
     */
    uint8_t value = 0;

    for (int bit = 0; bit < 8; bit++)
    {
        int idx = start + bit * 2;   // espacement approximatif
        if (idx >= BUF_SIZE)
            break;

        // Si le courant chute → bit = 1
        if (buf[idx] < threshold)
            value |= (1 << bit);
    }

    return value;
}

/* ============================================================
   Adresse détectée
   ============================================================ */
uint16_t EXSA_BoosterRailCom::getLastAddress()
{
    return _lastAddress;
}
