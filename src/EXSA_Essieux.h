/*
   EXSA_Essieux.h — Version EXSA universel (EXSA 2.0)
   Bruno + Copilot

   Module responsable de :
     - la gestion du capteur de présence (OCCUPÉ / LIBRE)
     - la gestion du mouvement ponctuel (PONCTUEL)
     - la réception des deltas d’essieux (+1 / -1) depuis la quadrature
     - l’envoi des trames correspondantes au SA (03 / 04 / 05)
     - la mise à jour des LEDs du canton (occupation + mouvement)

   Architecture EXSA 2.0 :
     - La quadrature est traitée via une queue FreeRTOS.
     - onDeltaAxe() est appelé dans la tâche EXSA (jamais en ISR).
     - updateCapteurPresence() et update() sont appelés depuis EXSA_Main::loop().
     - Le module est totalement statique : un seul canton par EXSA universel.
*/

#pragma once
#include <Arduino.h>

class EXSA_Essieux
{
public:

    /*
     * updateCapteurPresence(etat)
     * ---------------------------------------------------------
     * Appelé depuis EXSA_Main à chaque cycle.
     * Met à jour l’état OCCUPÉ / LIBRE du canton.
     * Envoie une trame au SA uniquement si l’état change.
     * Met également à jour les LEDs du canton.
     */
    static void updateCapteurPresence(bool etat) noexcept;

    /*
     * onDeltaAxe(delta)
     * ---------------------------------------------------------
     * Appelé par EXSA_Quadrature lorsqu’un essieu est détecté.
     * delta = +1 ou -1 selon le sens.
     *
     * Actions :
     *   - envoi immédiat au SA (trame 05)
     *   - pulse LED mouvement
     *   - activation du PONCTUEL (trame 03)
     *
     * Note :
     *   Cette fonction est appelée dans la tâche EXSA,
     *   jamais depuis une ISR.
     */
    static void onDeltaAxe(int delta) noexcept;

    /*
     * update()
     * ---------------------------------------------------------
     * Appelé régulièrement dans EXSA_Main::loop().
     * Gère l’extinction automatique du PONCTUEL après un délai
     * sans mouvement (timeout).
     */
    static void update() noexcept;

    /*
     * GETTERS (debug)
     * ---------------------------------------------------------
     * getPresenceEXSA() → état physique du capteur
     * getPresenceSA()   → dernier état envoyé au SA
     */
    [[nodiscard]] static bool getPresenceEXSA() noexcept;
    [[nodiscard]] static bool getPresenceSA()   noexcept;
};
