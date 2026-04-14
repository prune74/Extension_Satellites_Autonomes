/*
 * EXSA_Essieux.cpp — Version EXSA universel (EXSA 2.0)
 *
 * Gestion :
 *   - présence physique (capteur GPIO)
 *   - mouvement ponctuel (PONCTUEL)
 *   - comptage d’essieux (delta +1 / -1)
 *
 * Architecture :
 *   - updateCapteurPresence() appelé par EXSA_Main
 *   - onDeltaAxe() appelé par EXSA_Quadrature (tâche EXSA)
 *   - update() gère l’extinction automatique du PONCTUEL
 */

#include "EXSA_Essieux.h"
#include "EXSA_UartTx.h"
#include "EXSA_Config.h"
#include "EXSA_Canton.h"
#include <Arduino.h>

/* ============================================================
   Variables internes
   ============================================================ */

// Présence physique détectée par le capteur
static bool presencePhysique = false;

// Dernier état envoyé au SA (évite les doublons)
static bool presenceSA = false;

// Mouvement ponctuel actif
static bool ponctuelActif = false;

// Timestamp du dernier mouvement détecté
static unsigned long dernierMouvementMs = 0;

// Durée sans mouvement avant extinction du PONCTUEL
static constexpr unsigned long PONCTUEL_TIMEOUT_MS = 200;


/* ============================================================
   updateCapteurPresence()
   ------------------------------------------------------------
   Appelé depuis EXSA_Main.
   Met à jour OCCUPÉ / LIBRE et envoie la trame au SA
   uniquement en cas de changement.
   ============================================================ */
void EXSA_Essieux::updateCapteurPresence(bool etat) noexcept
{
    presencePhysique = etat;

#if EXSA_DEBUG
    Serial.printf("[EXSA] Presence capteur = %s\n",
                  etat ? "OCCUPE" : "LIBRE");
#endif

    // Envoi au SA uniquement si changement
    if (presencePhysique != presenceSA)
    {
        presenceSA = presencePhysique;

        // 1) Trame UART vers SA
        EXSA_UartTx::envoyerTrameOccupation(presenceSA);

        // 2) LED canton
        EXSA_Canton::setOccupation(presenceSA);
    }
}


/* ============================================================
   onDeltaAxe(delta)
   ------------------------------------------------------------
   Appelé par EXSA_Quadrature lorsqu’un essieu est détecté.
   delta = +1 ou -1.
   ============================================================ */
void EXSA_Essieux::onDeltaAxe(int delta) noexcept
{
    // 1) Envoi immédiat au SA
    EXSA_UartTx::envoyerTrameDeltaAxe(delta);

    // 2) LED mouvement locale
    EXSA_Canton::pulseMouvement();

    // 3) Activation du PONCTUEL si nécessaire
    if (!ponctuelActif)
    {
        ponctuelActif = true;
        EXSA_UartTx::envoyerTramePonctuel(true);
    }

    // Mise à jour du timestamp
    dernierMouvementMs = millis();
}


/* ============================================================
   update()
   ------------------------------------------------------------
   Appelé régulièrement dans EXSA_Main::loop().
   Gère l’extinction automatique du PONCTUEL.
   ============================================================ */
void EXSA_Essieux::update() noexcept
{
    const unsigned long now = millis();

    if (ponctuelActif && (now - dernierMouvementMs > PONCTUEL_TIMEOUT_MS))
    {
        ponctuelActif = false;
        EXSA_UartTx::envoyerTramePonctuel(false);
    }
}


/* ============================================================
   GETTERS (debug)
   ============================================================ */
bool EXSA_Essieux::getPresenceEXSA() noexcept { return presencePhysique; }
bool EXSA_Essieux::getPresenceSA()   noexcept { return presenceSA; }
