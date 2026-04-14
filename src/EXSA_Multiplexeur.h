/*
 * EXSA_Multiplexeur.h
 * Pilote Charlieplexing multi‑instance pour signaux ferroviaires
 * Version ESP32 — EXSA universel
 *
 * Ce module permet de piloter un mât SNCF complet (jusqu’à 9 LED)
 * en utilisant seulement 4 GPIO grâce au Charlieplexing.
 *
 * Le multiplexeur est indépendant :
 *   - EXSA_Signaux décide quelles LED doivent être allumées
 *   - EXSA_Multiplexeur applique l’état LED par LED
 *   - EXSA_Main appelle mettreAJour() régulièrement (≥ 200 Hz)
 */

#pragma once
#include <Arduino.h>

/*-------------------------------------------------------------
   IDs des LED du mât de signalisation (Charlieplexing)
   -------------------------------------------------------------
   ⚠️ IMPORTANT :
   Ces IDs DOIVENT correspondre EXACTEMENT aux index utilisés
   dans les tableaux ANODE[] et CATHODE[] du .cpp.
   Toute modification ici implique une mise à jour du .cpp.
--------------------------------------------------------------*/
enum ExsaLedId {
    LED_RALENTISSEMENT = 0,   // P1 -> P2
    LED_RAPPEL,               // P2 -> P1
    LED_SEMAPHORE,            // P2 -> P3
    LED_LIBRE,                // P3 -> P2
    LED_AVERTISSEMENT,        // P2 -> P4
    LED_OEILLETON,            // P4 -> P2
    LED_MANOEUVRE,            // P3 -> P4
    LED_CARRE,                // P4 -> P3
    LED_CARRE_VIOLET,         // P1 -> P4
    LED_MAX                   // nombre total de LEDs (toujours en dernier)
};

/*-------------------------------------------------------------
   Structure interne : état d'une LED
--------------------------------------------------------------*/
struct ExsaLedState {
    bool allumer;         // LED allumée ?
    uint8_t intensite;    // Intensité PWM (0–255)
    bool clignote;        // Mode clignotant activé ?
    bool etatClignote;    // État actuel ON/OFF du clignotement
};

/*-------------------------------------------------------------
   Classe EXSA_Multiplexeur
   -------------------------------------------------------------
   Gère un Charlieplexing 4×4 → 9 LEDs.
   Une seule LED est affichée à la fois, mais le scan rapide
   donne l’illusion d’un affichage simultané.
--------------------------------------------------------------*/
class EXSA_Multiplexeur {
public:

    /*
     * Constructeur
     * ---------------------------------------------------------
     * p1..p4 : les 4 GPIO utilisés pour le Charlieplexing.
     *
     * Chaque EXSA universel possède un seul multiplexeur.
     */
    EXSA_Multiplexeur(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);

    /*
     * réglerLed()
     * ---------------------------------------------------------
     * Configure une LED :
     *   - allumer / éteindre
     *   - intensité (0–255)
     *   - clignotement ON/OFF
     */
    void reglerLed(ExsaLedId id, bool allumer,
                   uint8_t intensite = 255,
                   bool clignote = false);

    /*
     * réglerIntensite()
     * ---------------------------------------------------------
     * Change uniquement l’intensité d’une LED.
     * Utile pour équilibrer visuellement le mât.
     */
    void reglerIntensite(ExsaLedId id, uint8_t intensite);

    /*
     * mettreAJour()
     * ---------------------------------------------------------
     * Fonction à appeler régulièrement dans loop().
     * Gère :
     *   - le clignotement (toggle ON/OFF)
     *   - le scan Charlieplexing (1 LED par cycle)
     *
     * Fréquence recommandée :
     *   - idéal : 1 kHz
     *   - minimum : 200 Hz (sinon scintillement visible)
     */
    void mettreAJour();

private:

    uint8_t P[4];                 // Pins physiques P1..P4
    ExsaLedState etats[LED_MAX];  // État de chaque LED

    uint32_t dernierCligno;       // Timer clignotement
    uint32_t dernierScan;         // Timer scan Charlieplexing
    uint8_t indexScan;            // LED actuellement affichée

    /*
     * configLigne()
     * ---------------------------------------------------------
     * Configure une broche :
     *   - INPUT (haute impédance)
     *   - OUTPUT LOW
     *   - OUTPUT HIGH
     *
     * Utilisé pour configurer anode/cathode à chaque scan.
     */
    void configLigne(uint8_t pin, int mode, int valeur = LOW);

    /*
     * appliquerLed()
     * ---------------------------------------------------------
     * Active une LED du mât :
     *   - toutes les broches passent d’abord en haute impédance
     *   - si la LED doit être allumée :
     *        * cathode = LOW
     *        * anode = PWM (analogWrite)
     */
    void appliquerLed(uint8_t index);
};
