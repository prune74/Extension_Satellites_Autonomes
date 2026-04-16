/*
 * EXSA_Main.cpp — Cœur logique EXSA 2.0 / 2026
 * ------------------------------------------------------------
 * Rôle :
 *   - Initialise tous les sous-modules EXSA :
 *        * UART (réception commandes SA)
 *        * PCA9685 (servos, direction, canton)
 *        * Charlieplexing (mât SNCF)
 *        * Quadrature (A/B → delta essieux)
 *        * Essieux (présence + ponctuel)
 *        * Switches (micro-switchs aiguilles)
 *
 *   - Reçoit les trames SA via EXSA_UartRx et appelle les callbacks
 *   - Met à jour les modules temps réel :
 *        * Signaux (clignotements + scan Charlieplexing)
 *        * Canton (mouvement, erreur, animation)
 *        * Essieux (logique ponctuelle + timeout)
 *        * Switches (position réelle aiguilles + sécurité)
 *
 *   - Ne contient aucune logique métier complexe :
 *        → tout est délégué aux modules spécialisés
 */

#include "EXSA_Main.h"
#include "EXSA_UartRx.h"
#include "EXSA_Signaux.h"
#include "EXSA_Direction.h"
#include "EXSA_Servo.h"
#include "EXSA_Canton.h"
#include "EXSA_Multiplexeur.h"
#include "EXSA_Quadrature.h"
#include "EXSA_Essieux.h"
#include "EXSA_Switches.h"
#include "EXSA_Config.h"

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "SA_EXSA_Protocol.h"

/* ============================================================
   Instance globale PCA9685
   ============================================================ */
Adafruit_PWMServoDriver g_pca9685 = Adafruit_PWMServoDriver();

/* ============================================================
   Variables globales EXSA (adressage)
   ============================================================ */

// 0 = EXSA côté H
// 1 = EXSA côté AH
uint8_t exsaAdresse = 0;

// true  = EXSA côté H
// false = EXSA côté AH
bool exsaIsHoraire = true;

/* ============================================================
   Instances globales EXSA
   ============================================================ */

// Multiplexeur Charlieplexing (1 mât, 4 GPIO définis dans EXSA_Config)
static EXSA_Multiplexeur mux(EXSA_MUX_P1, EXSA_MUX_P2, EXSA_MUX_P3, EXSA_MUX_P4);

// Mât SNCF (un seul mât pour EXSA universel)
static EXSA_Signaux signaux(&mux, EXSA_SIGNAL_EST_MANOEUVRE);

// Feux directionnels (4 LED max)
static EXSA_Direction direction(
    &g_pca9685,
    EXSA_DIR_1,
    EXSA_DIR_2,
    EXSA_DIR_3,
    EXSA_DIR_4
);

/* ============================================================
   Variables internes
   ============================================================ */
static uint8_t occupationVoisins = 0;
static uint8_t quadEtatPrecedent = 0;

/* ============================================================
   Initialisation complète EXSA
   ============================================================ */
void EXSA_Main::begin() noexcept
{
    if (EXSA_DEBUG)
        Serial.println("[EXSA] Initialisation...");

    /* -----------------------------
       Lecture du DIP H / AH
    ------------------------------*/
    pinMode(EXSA_DIP_PIN, INPUT_PULLUP);

    exsaAdresse   = digitalRead(EXSA_DIP_PIN) ? 0 : 1;
    exsaIsHoraire = (exsaAdresse == 0);

    if (EXSA_DEBUG)
        Serial.printf("[EXSA] Adresse = %s\n",
                      exsaIsHoraire ? "HORAIRE" : "ANTIHORAIRE");

    /* -----------------------------
       Initialisation capteur présence
    ------------------------------*/
    pinMode(EXSA_PRESENCE_PIN, INPUT);

    /* -----------------------------
       Initialisation UART
       (réception des trames SA → EXSA)
    ------------------------------*/
    EXSA_UartRx::begin(Serial1, EXSA_UART_BAUDRATE);

    /* -----------------------------
       PCA9685 (servos + canton + direction)
    ------------------------------*/
    EXSA_Servo::begin();
    EXSA_Canton::begin(&g_pca9685);

    /* -----------------------------
       MCP23017 (micro-switchs aiguilles)
       Sécurité + trame 0x06
    ------------------------------*/
    EXSA_Switches::begin();

    /* -----------------------------
       Quadrature : initialisation
    ------------------------------*/
    EXSA_Quadrature::initQueue();
    EXSA_Quadrature::installerInterruptions();
    quadEtatPrecedent = 0;

    /* -----------------------------
       Mât SNCF : aspect initial
    ------------------------------*/
    (void)signaux.setAspect(ASPECT_MASQUE);

    if (EXSA_DEBUG)
        Serial.println("[EXSA] OK");
}

/* ============================================================
   Boucle principale EXSA
   ============================================================ */
void EXSA_Main::loop() noexcept
{
    // Lecture UART (trames SA → EXSA)
    EXSA_UartRx::process();

    // Capteur de présence → Essieux (OCCUPÉ / LIBRE)
    {
        const bool presence = digitalRead(EXSA_PRESENCE_PIN);
        EXSA_Essieux::updateCapteurPresence(presence);
    }

    // Traitement quadrature (événements A/B → delta essieux)
    uint8_t nouvelEtat;
    while (EXSA_Quadrature::lireEvenement(nouvelEtat))
    {
        int8_t delta = 0;

        /*
         * Table quadrature EXSA 2.0
         * -------------------------
         * 00 → 01 → 11 → 10 → 00  = +1
         * 00 → 10 → 11 → 01 → 00  = -1
         */
        if ((quadEtatPrecedent == 0 && nouvelEtat == 1) ||
            (quadEtatPrecedent == 1 && nouvelEtat == 3) ||
            (quadEtatPrecedent == 3 && nouvelEtat == 2) ||
            (quadEtatPrecedent == 2 && nouvelEtat == 0))
        {
            delta = +1;
        }
        else if ((quadEtatPrecedent == 0 && nouvelEtat == 2) ||
                 (quadEtatPrecedent == 2 && nouvelEtat == 3) ||
                 (quadEtatPrecedent == 3 && nouvelEtat == 1) ||
                 (quadEtatPrecedent == 1 && nouvelEtat == 0))
        {
            delta = -1;
        }

        quadEtatPrecedent = nouvelEtat;

        if (delta != 0)
            EXSA_Essieux::onDeltaAxe(delta);
    }

    // Mise à jour logique Essieux (PONCTUEL / timeout)
    EXSA_Essieux::update();

    // Mise à jour du mât (clignotements + scan Charlieplexing)
    signaux.update();

    // Mise à jour LED canton (mouvement, erreurs…)
    EXSA_Canton::update();

    // Lecture micro-switchs + sécurité aiguilles + trame 0x06
    EXSA_Switches::update();
}

/* ============================================================
   Callbacks SA → EXSA
   ============================================================ */

void EXSA_Main::onTopologie(uint8_t *data, uint8_t len) noexcept
{
    if (EXSA_DEBUG)
        Serial.println("[EXSA] Topologie reçue");
}

void EXSA_Main::onConfigSignaux(uint8_t *data, uint8_t len) noexcept
{
    if (EXSA_DEBUG)
        Serial.println("[EXSA] Config signaux OK");
}

/* -----------------------------
   Aspects (E6 / E7)
------------------------------*/
void EXSA_Main::onAspectHoraire(uint8_t aspect) noexcept
{
    (void)signaux.setAspect((ExsaAspect)aspect);
}

void EXSA_Main::onAspectAntiHoraire(uint8_t aspect) noexcept
{
    (void)signaux.setAspect((ExsaAspect)aspect);
}

/* -----------------------------
   Direction (E8 / E9)
------------------------------*/
void EXSA_Main::onDirectionHoraire(uint8_t code) noexcept
{
    (void)direction.setDirection(code);
}

void EXSA_Main::onDirectionAntiHoraire(uint8_t code) noexcept
{
    (void)direction.setDirection(code);
}

/* -----------------------------
   Occupation voisins (EA)
------------------------------*/
void EXSA_Main::onOccupationVoisins(uint8_t value) noexcept
{
    occupationVoisins = value;
    EXSA_Canton::setVoisins(value);
}

/* -----------------------------
   Servos (F0 / F1 / F2)
------------------------------*/
void EXSA_Main::onServoMove(uint8_t servoIndex, uint8_t direction) noexcept
{
    // Commande servo → EXSA_Servo
    EXSA_Servo::move(servoIndex, direction);

    // Sécurité aiguilles : début mouvement
    EXSA_Switches::notifierMouvementDemarre(servoIndex);
}

void EXSA_Main::onServoConfig(uint8_t servoIndex,
                              uint16_t posDroit,
                              uint16_t posDevie,
                              uint16_t speed) noexcept
{
    EXSA_Servo::configure(servoIndex, posDroit, posDevie, speed);
}

void EXSA_Main::onServoTest(uint8_t servoIndex) noexcept
{
    EXSA_Servo::test(servoIndex);
}

/* -------------------------------------------------------------
   Fin de EXSA_Main.cpp
   -------------------------------------------------------------*/
