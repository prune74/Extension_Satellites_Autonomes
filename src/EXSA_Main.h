#pragma once
#include <Arduino.h>
#include "EXSA_Config.h"          // Configuration globale EXSA
#include "EXSA_Switches.h"        // ← AJOUT : sécurité aiguilles (micro-switchs)

/*
 * EXSA_Main.h — Version ENUM (Option A)
 * ------------------------------------------------------------
 * Cœur logique de l’EXSA universel.
 *
 * Rôle principal :
 *   - Initialiser tous les sous-modules EXSA
 *   - Recevoir et traiter les trames SA (via EXSA_UartRx)
 *   - Mettre à jour les modules temps réel
 *
 * Sous-modules initialisés :
 *   * UART (réception commandes SA)
 *   * PCA9685 (direction, canton, servos)
 *   * Charlieplexing (mât SNCF)
 *   * Quadrature (A/B → comptage essieux)
 *   * Essieux (présence, ponctuel, delta essieux)
 *   * Switches (micro-switchs aiguilles + sécurité)
 *
 * Commandes SA prises en charge :
 *   * Aspects SNCF (E6 / E7) → 1 octet (enum ExsaAspect)
 *   * Direction (E8 / E9)
 *   * Occupation voisins (EA)
 *   * Servos (F0 / F1 / F2)
 *
 * Mise à jour temps réel :
 *   * Charlieplexing (clignotements + scan)
 *   * LED canton (mouvement, erreur, occupation)
 *   * Quadrature (lecture queue FreeRTOS → delta essieux)
 *   * Essieux (présence + ponctuel)
 *   * Switches (position réelle aiguilles + trame 0x06)
 */

namespace EXSA_Main
{
    /* ========================================================
       Cycle de vie EXSA
       ======================================================== */

    void begin() noexcept;
    void loop() noexcept;

    /* ========================================================
       Callbacks SA → EXSA (déclenchés par EXSA_UartRx)
       ======================================================== */

    // Topologie (E4)
    void onTopologie(uint8_t *data, uint8_t len) noexcept;

    // Configuration signaux (E5)
    void onConfigSignaux(uint8_t *data, uint8_t len) noexcept;

    // Aspects (E6 / E7) — 1 octet (enum ExsaAspect)
    void onAspectHoraire(uint8_t aspect) noexcept;
    void onAspectAntiHoraire(uint8_t aspect) noexcept;

    // Direction (E8 / E9)
    void onDirectionHoraire(uint8_t code) noexcept;
    void onDirectionAntiHoraire(uint8_t code) noexcept;

    // Occupation voisins (EA)
    void onOccupationVoisins(uint8_t value) noexcept;

    // Servos (F0 / F1 / F2)
    void onServoMove(uint8_t servoIndex, uint8_t direction) noexcept;
    void onServoConfig(uint8_t servoIndex,
                       uint16_t posDroit,
                       uint16_t posDevie,
                       uint16_t speed) noexcept;
    void onServoTest(uint8_t servoIndex) noexcept;
}

/* ============================================================
   Fin de EXSA_Main.h — Version ENUM
   ============================================================ */
