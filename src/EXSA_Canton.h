#pragma once
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

/*
 * EXSA_Canton.h
 * ------------------------------------------------------------
 * Module responsable de l’affichage de l’état d’un canton :
 *
 *   - OCCUPÉ      (rouge)
 *   - LIBRE       (vert)
 *   - MOUVEMENT   (orange, pulse court lors du passage d’un essieu)
 *   - ERREUR      (rouge fixe ou clignotant selon EXSA_Main)
 *
 * Toutes les LEDs sont pilotées via le PCA9685 (PWM 0–4095).
 *
 * Le module est entièrement statique :
 *   → un seul canton par EXSA universel
 *   → aucune instance à créer
 *   → toutes les fonctions sont statiques
 *
 * EXSA_Main appelle :
 *   - begin()       au démarrage
 *   - update()      dans loop()
 *   - setOccupation(), pulseMouvement(), setErreur() selon les événements
 */

class EXSA_Canton {
public:

    /*
     * begin()
     * ---------------------------------------------------------
     * Appelé par EXSA_Main au démarrage.
     * Initialise le module via initialiser().
     */
    static void begin(Adafruit_PWMServoDriver* driver) noexcept;

    /*
     * initialiser()
     * ---------------------------------------------------------
     * Initialise le module canton :
     *   - enregistre le pointeur PCA9685
     *   - éteint toutes les LEDs
     *   - lance l’animation de démarrage (séquence 4 LEDs)
     *
     * À appeler uniquement via begin().
     */
    static void initialiser(Adafruit_PWMServoDriver* driver) noexcept;

    /*
     * update()
     * ---------------------------------------------------------
     * Fonction à appeler régulièrement dans EXSA_Main::loop().
     * Gère :
     *   - l’animation de démarrage
     *   - le pulse de mouvement (LED orange)
     */
    static void update() noexcept;

    /*
     * setOccupation(occupe)
     * ---------------------------------------------------------
     * Met à jour l’état OCCUPÉ / LIBRE du canton.
     *   - OCCUPÉ = rouge ON, vert OFF
     *   - LIBRE  = vert ON, rouge OFF
     *
     * Ignoré tant que l’animation de démarrage n’est pas terminée.
     */
    static void setOccupation(bool occupe) noexcept;

    /*
     * pulseMouvement()
     * ---------------------------------------------------------
     * Active la LED de mouvement pendant EXSA_CANTON_MOUVEMENT_MS.
     * Appelé par EXSA_Essieux lorsqu’un essieu est détecté.
     */
    static void pulseMouvement() noexcept;

    /*
     * setErreur(erreur)
     * ---------------------------------------------------------
     * Active ou désactive la LED d’erreur.
     * Ignoré tant que l’animation de démarrage n’est pas terminée.
     */
    static void setErreur(bool erreur) noexcept;

    /*
     * setVoisins(voisins)
     * ---------------------------------------------------------
     * Appelé par EXSA_Main lors de la réception EA.
     * Réservé pour extensions futures (EXSA 2.1).
     */
    static void setVoisins(uint8_t voisins) noexcept;

#if EXSA_DEBUG
    /*
     * debugCapteurs()
     * ---------------------------------------------------------
     * Mode debug : permet d’allumer directement les LEDs du canton.
     * Très utile pour tester le câblage ou le PCA9685.
     */
    static void debugCapteurs(bool occupe, bool libre, bool mouv, bool err) noexcept;
#endif

    /*
     * pca
     * ---------------------------------------------------------
     * Pointeur vers le PCA9685 utilisé pour piloter les LEDs.
     * Rendu public pour permettre aux helpers internes (ledOn/ledOff)
     * d’y accéder sans casser l’architecture statique.
     */
    static Adafruit_PWMServoDriver* pca;
};
/* ============================================================
   Fin de EXSA_Canton.h
   ============================================================ */