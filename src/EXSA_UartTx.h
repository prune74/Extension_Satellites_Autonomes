#pragma once
#include <Arduino.h>

/*
 * EXSA_UartTx.h
 * ------------------------------------------------------------
 * Gestion des trames UART envoyées par l’EXSA vers le SA.
 *
 * Format général d’une trame :
 *   [0] SYNC   = 0xAA (PROTO_SYNC_BYTE)
 *   [1] OPCODE = type de trame (03, 04, 05, 06…)
 *   [2] DATA   = valeur associée (selon le type)
 *
 * Ce module est entièrement statique :
 *   - aucune instance à créer
 *   - utilisé par EXSA_Essieux, EXSA_Switches et EXSA_Main
 *
 * Rôle :
 *   - informer le SA de l’état du canton (OCCUPÉ / LIBRE)
 *   - envoyer les deltas essieux (+1 / -1)
 *   - signaler l’état PONCTUEL (H/AH)
 *   - transmettre la position réelle des aiguilles (micro-switchs)
 *   - répondre au PING du SA (PONG)
 */

class EXSA_UartTx
{
public:

    /*
     * envoyerPong()
     * ---------------------------------------------------------
     * Répond au PING envoyé par le SA.
     *
     * EXSA reste 100 % passif :
     *   → il ne parle jamais en premier
     *   → il répond uniquement si le SA envoie PROTO_PING (0x30)
     *
     * Trame envoyée :
     *   [0xAA][0x31][index]
     *
     * index = identifiant EXSA (0 = H, 1 = AH)
     */
    static void envoyerPong(uint8_t index);

    /*
     * envoyerTrameOccupation()
     * ---------------------------------------------------------
     * Envoie l’état OCCUPÉ / LIBRE du canton au SA.
     *
     *   occ = true  → PROTO_OCC_ACTIVE  (0x30)
     *   occ = false → PROTO_OCC_LIBRE   (0x31)
     *
     * Trame envoyée :
     *   [0xAA][0x04][0x30/0x31]
     */
    static void envoyerTrameOccupation(bool occ);

    /*
     * envoyerTrameDeltaAxe()
     * ---------------------------------------------------------
     * Envoie un delta d’essieu (+1 ou -1) au SA.
     *
     *   delta > 0 → PROTO_DELTA_PLUS_UN  (0x01)
     *   delta < 0 → PROTO_DELTA_MOINS_UN (0xFF)
     *
     * Trame envoyée :
     *   [0xAA][0x05][0x01/0xFF]
     */
    static void envoyerTrameDeltaAxe(int delta);

    /*
     * envoyerTramePonctuel()
     * ---------------------------------------------------------
     * Envoie l’état PONCTUEL (mouvement détecté) au SA.
     *
     * Le code dépend du côté EXSA :
     *   - côté H  → 0x10 (actif) / 0x11 (inactif)
     *   - côté AH → 0x12 (actif) / 0x13 (inactif)
     *
     * Trame envoyée :
     *   [0xAA][0x03][0x10/0x11/0x12/0x13]
     */
    static void envoyerTramePonctuel(bool actif);

    /*
     * envoyerTramePositionAiguille()
     * ---------------------------------------------------------
     * Envoie au SA la position réelle d’une aiguille, déterminée
     * par les micro-switchs lus via MCP23017.
     *
     * Paramètres :
     *   index    = numéro d’aiguille (0..5)
     *   position = position réelle :
     *                0 = DROIT
     *                1 = DEVIE
     *                2 = INDET (aucun switch actif)
     *                3 = INCOHERENT (les deux actifs)
     *
     *   etat     = état sécurité :
     *                0 = OK
     *                1 = BLOQUE (mouvement trop long)
     *                2 = ERREUR (incohérence persistante)
     *
     * Trame envoyée :
     *   [0xAA][0x06][index][position][etat]
     *
     * Cette trame est envoyée :
     *   → immédiatement si la position réelle change
     *   → périodiquement (200 ms) pour supervision continue
     */
    static void envoyerTramePositionAiguille(uint8_t index,
                                             uint8_t position,
                                             uint8_t etat);
};

/* ============================================================
   Fin de EXSA_UartTx.h — Version PING/PONG
   ============================================================ */
