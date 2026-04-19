#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_BoosterRailCom.h
 * ------------------------------------------------------------
 *  Module RailCom Discovery 2026 — Décodage Locoduino
 *
 *  Rôle :
 *    - gérer l’échantillonnage ADC haute fréquence (~83 kHz)
 *    - capturer la fenêtre RailCom pendant le cutout
 *    - découper CH1 / CH2 (méthode Locoduino)
 *    - décoder les 8 bits de chaque canal
 *    - reconstruire les adresses courtes / longues
 *    - filtrer les parasites (4 valeurs identiques)
 *
 *  Ce module NE gère PAS :
 *    - le cutout (géré par EXSA_BoosterHw)
 *    - la sécurité (gérée par EXSA_Booster)
 *    - le CAN (géré par EXSA_CanBooster)
 *
 *  Il fournit simplement :
 *      getLastAddress()
 *      clearLastAddress()
 *
 *  L’ISR HF remplit un buffer brut.
 *  Le décodage est lancé à la fin du cutout.
 * ============================================================
 */

class EXSA_BoosterRailCom
{
public:
    /*
     * --------------------------------------------------------
     * begin()
     * --------------------------------------------------------
     * Initialise :
     *   - les variables internes
     *   - le timer matériel (1 MHz → 83 kHz)
     *   - l’ISR d’échantillonnage ADC
     *
     * Appelé une seule fois dans EXSA_Booster::begin().
     */
    static void begin();

    /*
     * --------------------------------------------------------
     * onCutoutStart()
     * --------------------------------------------------------
     * Appelé au début du cutout RailCom.
     *   - reset du buffer
     *   - activation de l’ISR HF
     */
    static void onCutoutStart();

    /*
     * --------------------------------------------------------
     * onCutoutEnd()
     * --------------------------------------------------------
     * Appelé à la fin du cutout.
     *   - désactive l’ISR HF
     *   - lance le décodage si assez d’échantillons
     */
    static void onCutoutEnd();

    /*
     * --------------------------------------------------------
     * update()
     * --------------------------------------------------------
     * Hook optionnel (appelé toutes les 1 ms).
     * Actuellement inutilisé, réservé debug.
     */
    static void update();

    /*
     * --------------------------------------------------------
     * getLastAddress()
     * clearLastAddress()
     * --------------------------------------------------------
     * Accès à la dernière adresse RailCom stable.
     * 0 = aucune adresse valide.
     */
    static uint16_t getLastAddress();
    static void     clearLastAddress();

    /*
     * --------------------------------------------------------
     * Variables accessibles par l’ISR
     * --------------------------------------------------------
     * _active :
     *    true  = l’ISR stocke les échantillons
     *    false = l’ISR ignore les appels
     *
     * _index :
     *    nombre d’échantillons capturés
     *
     * _buffer :
     *    buffer ADC haute fréquence (brut)
     */
    static volatile bool   _active;
    static volatile int    _index;
    static const int       BUF_SIZE = 128;
    static int16_t         _buffer[BUF_SIZE];

private:
    /*
     * --------------------------------------------------------
     * _lastAddress
     * --------------------------------------------------------
     * Dernière adresse RailCom stable (filtrée).
     * Mise à jour uniquement par decode().
     */
    static uint16_t _lastAddress;

    /*
     * --------------------------------------------------------
     * decode()
     * --------------------------------------------------------
     * Décodage complet Locoduino :
     *   - découpage CH1 / CH2
     *   - décodage 8 bits
     *   - reconstruction adresse courte / longue
     *   - filtrage anti-parasites
     */
    static void decode();

    /*
     * --------------------------------------------------------
     * decodeChannel()
     * --------------------------------------------------------
     * Décodage d’un canal RailCom (8 bits) :
     *   - seuil dynamique min/max
     *   - recherche front descendant
     *   - échantillonnage centré
     */
    static uint8_t decodeChannel(const int16_t *buf,
                                 int startIndex,
                                 int length);
};
