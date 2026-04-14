/*
 * EXSA_UartTx.cpp
 * ------------------------------------------------------------
 * Gestion des trames envoyées par l’EXSA vers le SA.
 *
 * Format général d’une trame :
 *   [0] SYNC  = 0xAA
 *   [1] OPCODE (03, 04, 05…)
 *   [2] DATA  (selon le type)
 *
 * Ce module est entièrement statique :
 *   → aucune instance à créer
 *   → utilisé directement par EXSA_Essieux et EXSA_Main
 */

#include "EXSA_UartTx.h"
#include "SA_EXSA_Protocol.h"
#include "EXSA_Config.h"
#include <HardwareSerial.h>

// UART utilisé pour communiquer avec le SA (déclaré dans EXSA_Main.cpp)
extern HardwareSerial Serial1;

// Adresse EXSA (H/AH) définie dans EXSA_Main.cpp
extern bool exsaIsHoraire;

/* ============================================================
   envoyerPong()
   ------------------------------------------------------------
   Réponse au PING envoyé par le SA.
   EXSA reste 100 % passif : il ne parle jamais en premier.
   Il répond uniquement si le SA envoie PROTO_PING.
============================================================ */
void EXSA_UartTx::envoyerPong(uint8_t index)
{
#if EXSA_DEBUG
    Serial.printf("[EXSA TX] PONG (index=%u)\n", index);
#endif

    Serial1.write(PROTO_SYNC_BYTE);
    Serial1.write(PROTO_PONG);   // 0x31
    Serial1.write(index);        // identifiant EXSA (0 = H, 1 = AH)
}

/* ============================================================
   envoyerTrameOccupation()
   ------------------------------------------------------------
   Envoie l’état OCCUPÉ / LIBRE du canton vers le SA.
   Utilisé par EXSA_Essieux lorsque la présence change.
============================================================ */
void EXSA_UartTx::envoyerTrameOccupation(bool occ)
{
    const uint8_t valeur = occ ? PROTO_OCC_ACTIVE
                               : PROTO_OCC_LIBRE;

#if EXSA_DEBUG
    Serial.printf("[EXSA TX] OCCUPATION = %s (0x%02X)\n",
                  occ ? "OCCUPE" : "LIBRE",
                  valeur);
#endif

    Serial1.write(PROTO_SYNC_BYTE);
    Serial1.write(PROTO_04_OCCUPATION);
    Serial1.write(valeur);
}

/* ============================================================
   envoyerTrameDeltaAxe()
   ------------------------------------------------------------
   Envoie un delta essieu :
     +1 → PROTO_DELTA_PLUS_UN  (0x01)
     -1 → PROTO_DELTA_MOINS_UN (0xFF)
   Appelé par EXSA_Essieux lors d’un mouvement quadrature.
============================================================ */
void EXSA_UartTx::envoyerTrameDeltaAxe(int delta)
{
    const uint8_t valeur = (delta > 0) ? PROTO_DELTA_PLUS_UN
                                       : PROTO_DELTA_MOINS_UN;

#if EXSA_DEBUG
    Serial.printf("[EXSA TX] DELTA AXE = %d (0x%02X)\n", delta, valeur);
#endif

    Serial1.write(PROTO_SYNC_BYTE);
    Serial1.write(PROTO_05_DELTA_AXE);
    Serial1.write(valeur);
}

/* ============================================================
   envoyerTramePonctuel()
   ------------------------------------------------------------
   Envoie l’état PONCTUEL (actif/inactif) vers le SA.
   Le code dépend du côté EXSA :
     - H  → PROTO_PONCT_H_ACTIVE / INACTIVE
     - AH → PROTO_PONCT_AH_ACTIVE / INACTIVE
============================================================ */
void EXSA_UartTx::envoyerTramePonctuel(bool actif)
{
    uint8_t valeur;

    if (exsaIsHoraire)
        valeur = actif ? PROTO_PONCT_H_ACTIVE : PROTO_PONCT_H_INACTIVE;
    else
        valeur = actif ? PROTO_PONCT_AH_ACTIVE : PROTO_PONCT_AH_INACTIVE;

#if EXSA_DEBUG
    Serial.printf("[EXSA TX] PONCTUEL = %s (0x%02X)\n",
                  actif ? "ACTIF" : "INACTIF",
                  valeur);
#endif

    Serial1.write(PROTO_SYNC_BYTE);
    Serial1.write(PROTO_03_PONCTUEL);
    Serial1.write(valeur);
}

/* ============================================================
   envoyerTramePositionAiguille()
   ------------------------------------------------------------
   Envoie au SA la position réelle d’une aiguille, déterminée
   par les micro-switchs lus via MCP23017.

   Format de la trame :
     [0] SYNC  = 0xAA
     [1] 0x06  = PROTO_06_POSITION_AIGUILLE
     [2] index = numéro d’aiguille (0..5)
     [3] pos   = position réelle :
                   0 = DROIT
                   1 = DEVIE
                   2 = INDET (aucun switch actif)
                   3 = INCOHERENT (les deux actifs)
     [4] etat  = état sécurité :
                   0 = OK
                   1 = BLOQUE (mouvement trop long)
                   2 = ERREUR (incohérence persistante)

   Cette trame est envoyée :
     → immédiatement si la position réelle change
     → périodiquement (200 ms) pour supervision continue
============================================================ */
void EXSA_UartTx::envoyerTramePositionAiguille(uint8_t index,
                                               uint8_t position,
                                               uint8_t etat)
{
#if EXSA_DEBUG
    Serial.printf("[EXSA TX] POS AIG %u : pos=%u etat=%u\n",
                  index, position, etat);
#endif

    Serial1.write(PROTO_SYNC_BYTE);
    Serial1.write(PROTO_06_POSITION_AIGUILLE);
    Serial1.write(index);
    Serial1.write(position);
    Serial1.write(etat);
}

/* ============================================================
   Fin de EXSA_UartTx.cpp — Version PING/PONG
   ============================================================ */
