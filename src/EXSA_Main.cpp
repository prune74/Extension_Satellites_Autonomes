/*
 * EXSA_Main.cpp — Cœur logique EXSA 2.0 / 2026
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
#include "EXSA_Pins.h"

#include "EXSA_Booster.h"   // toujours inclus, mais activé dynamiquement

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
uint8_t exsaAdresse = 0;     // 0 = H, 1 = AH
bool exsaIsHoraire = true;   // true = H, false = AH
bool exsaHasBooster = false; // DIP Booster

/* ============================================================
   Instances globales EXSA
   ============================================================ */
static EXSA_Multiplexeur mux(EXSA_MUX_P1, EXSA_MUX_P2, EXSA_MUX_P3, EXSA_MUX_P4);
static EXSA_Signaux signaux(&mux, EXSA_SIGNAL_EST_MANOEUVRE);
static EXSA_Direction direction(&g_pca9685, EXSA_DIR_1, EXSA_DIR_2, EXSA_DIR_3, EXSA_DIR_4);

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
       DIP H/AH
    ------------------------------*/
    pinMode(EXSA_DIP_HAH_PIN, INPUT_PULLUP);
    exsaIsHoraire = (digitalRead(EXSA_DIP_HAH_PIN) == HIGH);
    exsaAdresse   = exsaIsHoraire ? 0 : 1;

    if (EXSA_DEBUG)
        Serial.printf("[EXSA] Côté = %s\n",
                      exsaIsHoraire ? "HORAIRE" : "ANTIHORAIRE");

    /* -----------------------------
       DIP Booster ON/OFF
    ------------------------------*/
    pinMode(EXSA_DIP_BOOSTER_PIN, INPUT_PULLUP);
    exsaHasBooster = (digitalRead(EXSA_DIP_BOOSTER_PIN) == HIGH);

    if (EXSA_DEBUG)
        Serial.printf("[EXSA] Booster = %s\n",
                      exsaHasBooster ? "ACTIF" : "INACTIF");

    /* -----------------------------
       Capteur présence
    ------------------------------*/
    pinMode(EXSA_PRESENCE_PIN, INPUT);

    /* -----------------------------
       UART SA → EXSA
    ------------------------------*/
    EXSA_UartRx::begin(Serial1, EXSA_UART_BAUDRATE);

    /* -----------------------------
       PCA9685
    ------------------------------*/
    EXSA_Servo::begin();
    EXSA_Canton::begin(&g_pca9685);

    /* -----------------------------
       MCP23017 (micro-switchs)
    ------------------------------*/
    EXSA_Switches::begin();

    /* -----------------------------
       Quadrature
    ------------------------------*/
    EXSA_Quadrature::initQueue();
    EXSA_Quadrature::installerInterruptions();
    quadEtatPrecedent = 0;

    /* -----------------------------
       Mât SNCF
    ------------------------------*/
    (void)signaux.setAspect(ASPECT_MASQUE);

    /* -----------------------------
       Booster (si DIP actif)
    ------------------------------*/
    if (exsaHasBooster)
    {
        EXSA_Booster::begin();
    }

    if (EXSA_DEBUG)
        Serial.println("[EXSA] OK");
}

/* ============================================================
   Boucle principale EXSA
   ============================================================ */
void EXSA_Main::loop() noexcept
{
    EXSA_UartRx::process();

    // Présence
    {
        const bool presence = digitalRead(EXSA_PRESENCE_PIN);
        EXSA_Essieux::updateCapteurPresence(presence);
    }

    // Quadrature
    uint8_t nouvelEtat;
    while (EXSA_Quadrature::lireEvenement(nouvelEtat))
    {
        int8_t delta = 0;

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

    EXSA_Essieux::update();
    signaux.update();
    EXSA_Canton::update();
    EXSA_Switches::update();

    // Booster dynamique
    if (exsaHasBooster)
    {
        EXSA_Booster::update();
    }
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

void EXSA_Main::onAspectHoraire(uint8_t aspect) noexcept
{
    (void)signaux.setAspect((ExsaAspect)aspect);
}

void EXSA_Main::onAspectAntiHoraire(uint8_t aspect) noexcept
{
    (void)signaux.setAspect((ExsaAspect)aspect);
}

void EXSA_Main::onDirectionHoraire(uint8_t code) noexcept
{
    (void)direction.setDirection(code);
}

void EXSA_Main::onDirectionAntiHoraire(uint8_t code) noexcept
{
    (void)direction.setDirection(code);
}

void EXSA_Main::onOccupationVoisins(uint8_t value) noexcept
{
    occupationVoisins = value;
    EXSA_Canton::setVoisins(value);
}

void EXSA_Main::onServoMove(uint8_t servoIndex, uint8_t direction) noexcept
{
    EXSA_Servo::move(servoIndex, direction);
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
