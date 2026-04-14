#pragma once
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "SA_EXSA_Protocol.h"
#include "EXSA_Pins.h"
#include "EXSA_Config.h"

/*
 * ============================================================
 *  EXSA_Switches.h — Version finale 2026
 * ------------------------------------------------------------
 *  Module de lecture des micro-switchs de position réelle
 *  des aiguilles via MCP23017 + sécurité complète.
 *
 *  Rôle :
 *    - Lire les contacts DROIT / DEVIE (2 par aiguille)
 *    - Déduire la position réelle (DROIT / DEVIE / INDET / INCOH)
 *    - Déduire l’état sécurité (OK / BLOQUE / ERREUR)
 *    - Détection blocage basée sur :
 *         → début mouvement (notifié par EXSA_Main)
 *         → temps max calculé via PWM (EXSA_Servo)
 *    - Envoyer la trame 0x06 au SA :
 *         → sur changement immédiat
 *         → périodiquement (200 ms)
 *
 *  Séparation des responsabilités :
 *    - EXSA_Servo : actionneur (commande)
 *    - EXSA_Switches : contrôle (sécurité)
 * ============================================================
 */

class EXSA_Switches
{
public:
    // Initialisation du MCP23017 (I²C + configuration des entrées)
    static void begin();

    // Lecture périodique (appelée dans loop() toutes les 10–20 ms)
    static void update();

    /*
     * Appelé par EXSA_Main lorsqu’un F0 est reçu.
     * Permet d’activer la surveillance de blocage.
     */
    static void notifierMouvementDemarre(uint8_t idx);

private:
    // Instance MCP23017 (16 entrées/sorties)
    static Adafruit_MCP23X17 mcp;

    // Nombre d’aiguilles gérées par EXSA
    static constexpr uint8_t AIG_COUNT = 3;

    /*
     * Mapping interne :
     *   Chaque aiguille utilise 2 entrées MCP23017 :
     *     - swDroit[i] = contact DROIT
     *     - swDevie[i] = contact DEVIE
     *
     *  ⚠️ IMPORTANT — ESP32 / GCC 8.4.0 :
     *     Un tableau static constexpr DOIT avoir un initialiseur
     *     dans le header, sinon le compilateur refuse.
     *
     *     Comme nous voulons définir les valeurs dans le .cpp,
     *     nous utilisons donc :
     *
     *         static const uint8_t [...]
     *
     *     → const = OK sans initialiseur
     *     → définition dans EXSA_Switches.cpp
     */
    static const uint8_t swDroit[AIG_COUNT];
    static const uint8_t swDevie[AIG_COUNT];

    /*
     * Mémorisation des derniers états envoyés au SA
     * pour détecter les changements + envoi périodique.
     */
    static uint8_t  lastPos[AIG_COUNT];
    static uint8_t  lastEtat[AIG_COUNT];
    static uint32_t lastSendMs[AIG_COUNT];

    /*
     * Détection de blocage :
     *   - movementActive[i] = le servo est censé bouger
     *   - moveStartMs[i]    = début du mouvement
     *   - moveMaxMs[i]      = durée max autorisée (calculée via PWM)
     */
    static uint32_t moveStartMs[AIG_COUNT];
    static uint32_t moveMaxMs[AIG_COUNT];
    static bool     movementActive[AIG_COUNT];

    // Lecture brute des micro-switchs et déduction position réelle
    static uint8_t lirePosition(uint8_t idx);

    // Déduction de l’état sécurité (OK / BLOQUE / ERREUR)
    static uint8_t lireEtat(uint8_t idx, uint8_t pos);

    // Envoi trame 0x06 au SA
    static void envoyerTrame(uint8_t idx, uint8_t pos, uint8_t etat);
};

/* ============================================================
   Fin de EXSA_Switches.h
   ============================================================ */
