/*
 * EXSA_Canton.cpp
 * ------------------------------------------------------------
 * Gestion des LEDs d’état du canton via PCA9685.
 *
 * LEDs utilisées :
 *   - OCCUPÉ      (rouge)
 *   - LIBRE       (vert)
 *   - MOUVEMENT   (orange, pulse court)
 *   - ERREUR      (rouge clignotant ou fixe selon EXSA_Main)
 *
 * Le module gère :
 *   - une animation de démarrage (séquence 4 LEDs)
 *   - un pulse de mouvement (durée EXSA_CANTON_MOUVEMENT_MS)
 *   - l’affichage OCCUPÉ / LIBRE
 *   - l’affichage ERREUR
 *
 * Le PCA9685 est partagé avec EXSA_Servo et EXSA_Direction.
 */

#include "EXSA_Canton.h"
#include "EXSA_Config.h"

// -----------------------------------------------------------------------------
// Pointeur PCA9685 partagé (défini dans EXSA_Main)
// -----------------------------------------------------------------------------
Adafruit_PWMServoDriver* EXSA_Canton::pca = nullptr;

// -----------------------------------------------------------------------------
// Variables internes (animation + mouvement)
// -----------------------------------------------------------------------------
static uint8_t  animStep      = 0;
static unsigned long animTimer = 0;
static bool animEnCours       = true;

static bool mouvementActif    = false;
static unsigned long mouvementTimer = 0;

// -----------------------------------------------------------------------------
// Helpers PCA (inline = plus rapide)
// -----------------------------------------------------------------------------
static inline void ledOff(uint8_t ch) noexcept {
    if (EXSA_Canton::pca)
        EXSA_Canton::pca->setPWM(ch, 0, 0);
}

static inline void ledOn(uint8_t ch, uint16_t pwm = 4095) noexcept {
    if (EXSA_Canton::pca)
        EXSA_Canton::pca->setPWM(ch, 0, pwm);
}

// -----------------------------------------------------------------------------
// begin() — appelé par EXSA_Main
// -----------------------------------------------------------------------------
void EXSA_Canton::begin(Adafruit_PWMServoDriver* driver) noexcept
{
    initialiser(driver);
}

// -----------------------------------------------------------------------------
// Compatibilité EXSA_Main : EA (voisins)
// -----------------------------------------------------------------------------
void EXSA_Canton::setVoisins(uint8_t v) noexcept
{
    // Pour l’instant, EXSA_Canton n’utilise pas cette info.
    (void)v;
}

// -----------------------------------------------------------------------------
// Initialisation interne
// -----------------------------------------------------------------------------
void EXSA_Canton::initialiser(Adafruit_PWMServoDriver* driver) noexcept {

    pca = driver;

    if (!pca)
        return;

    // Éteindre toutes les LEDs
    ledOff(EXSA_CANTON_OCCUPE_CH);
    ledOff(EXSA_CANTON_LIBRE_CH);
    ledOff(EXSA_CANTON_MOUVEMENT_CH);
    ledOff(EXSA_CANTON_ERREUR_CH);

    // Animation de démarrage
    animStep  = 0;
    animTimer = millis();
    animEnCours = true;
}

// -----------------------------------------------------------------------------
// update() — animation + pulse mouvement
// -----------------------------------------------------------------------------
void EXSA_Canton::update() noexcept {

    const unsigned long now = millis();

    // ---------------------------------------------------------
    // Animation de démarrage (séquence OCCUPÉ → LIBRE → MVT → ERR)
    // ---------------------------------------------------------
    if (animEnCours) {
        if (now - animTimer > EXSA_CANTON_ANIM_STEP_MS) {
            animTimer = now;

            // Tout OFF avant d’allumer l’étape suivante
            ledOff(EXSA_CANTON_OCCUPE_CH);
            ledOff(EXSA_CANTON_LIBRE_CH);
            ledOff(EXSA_CANTON_MOUVEMENT_CH);
            ledOff(EXSA_CANTON_ERREUR_CH);

            switch (animStep) {
                case 0: ledOn(EXSA_CANTON_OCCUPE_CH);     break;
                case 1: ledOn(EXSA_CANTON_LIBRE_CH);      break;
                case 2: ledOn(EXSA_CANTON_MOUVEMENT_CH);  break;
                case 3: ledOn(EXSA_CANTON_ERREUR_CH);     break;

                case 4:
                    // Fin animation → état initial = LIBRE
                    ledOn(EXSA_CANTON_LIBRE_CH);
                    animEnCours = false;
                    break;
            }

            if (animStep < 4)
                animStep++;
        }
    }

    // ---------------------------------------------------------
    // Pulse mouvement (LED orange ON pendant X ms)
    // ---------------------------------------------------------
    if (mouvementActif && now - mouvementTimer > EXSA_CANTON_MOUVEMENT_MS) {
        mouvementActif = false;
        ledOff(EXSA_CANTON_MOUVEMENT_CH);
    }
}

// -----------------------------------------------------------------------------
// OCCUPÉ / LIBRE
// -----------------------------------------------------------------------------
void EXSA_Canton::setOccupation(bool occupe) noexcept {

    if (!animEnCours) {

        // OCCUPÉ = rouge ON, vert OFF
        ledOn(EXSA_CANTON_OCCUPE_CH, occupe ? 4095 : 0);

        // LIBRE = vert ON, rouge OFF
        ledOn(EXSA_CANTON_LIBRE_CH, occupe ? 0 : 4095);
    }
}

// -----------------------------------------------------------------------------
// Pulse mouvement
// -----------------------------------------------------------------------------
void EXSA_Canton::pulseMouvement() noexcept {
    mouvementActif = true;
    mouvementTimer = millis();
    ledOn(EXSA_CANTON_MOUVEMENT_CH);
}

// -----------------------------------------------------------------------------
// ERREUR
// -----------------------------------------------------------------------------
void EXSA_Canton::setErreur(bool erreur) noexcept {
    if (!animEnCours) {
        ledOn(EXSA_CANTON_ERREUR_CH, erreur ? 4095 : 0);
    }
}

// -----------------------------------------------------------------------------
// Debug (affichage direct des 4 LED)
// -----------------------------------------------------------------------------
#if EXSA_DEBUG
void EXSA_Canton::debugCapteurs(bool occupe, bool libre, bool mouv, bool err) noexcept {
    ledOn(EXSA_CANTON_OCCUPE_CH,     occupe ? 4095 : 0);
    ledOn(EXSA_CANTON_LIBRE_CH,      libre  ? 4095 : 0);
    ledOn(EXSA_CANTON_MOUVEMENT_CH,  mouv   ? 4095 : 0);
    ledOn(EXSA_CANTON_ERREUR_CH,     err    ? 4095 : 0);
}
#endif

/* -------------------------------------------------------------
   Fin de EXSA_Canton.cpp
   -------------------------------------------------------------*/
