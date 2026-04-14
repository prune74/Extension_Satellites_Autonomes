/*
 * EXSA_Direction.cpp
 *
 * Gestion des feux directionnels (jusqu’à 4 LED) via PCA9685.
 *
 * Chaque LED correspond à un canal PCA9685.
 * Le module gère :
 *   - l’état ON/OFF de chaque LED
 *   - l’intensité individuelle (0–255 → 0–4095)
 *   - l’affichage d’un “niveau directionnel” (0 à 4)
 *
 * EXSA_Main appelle setDirection() lorsqu’il reçoit E8/E9.
 */

#include "EXSA_Direction.h"
#include "EXSA_Config.h"

/*-------------------------------------------------------------
   Constructeur
   -------------------------------------------------------------
   driver : pointeur vers le PCA9685 partagé
   c1..c4 : canaux PCA9685 utilisés pour les LED directionnelles
--------------------------------------------------------------*/
EXSA_Direction::EXSA_Direction(Adafruit_PWMServoDriver* driver,
                               uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) noexcept
    : pca(driver)
{
    C[0] = c1;
    C[1] = c2;
    C[2] = c3;
    C[3] = c4;

    // États initiaux
    etats[0] = false;
    etats[1] = false;
    etats[2] = false;
    etats[3] = false;

    // Intensités configurables (0–255)
    intensites[0] = EXSA_DIR_LED0_INTENSITE;
    intensites[1] = EXSA_DIR_LED1_INTENSITE;
    intensites[2] = EXSA_DIR_LED2_INTENSITE;
    intensites[3] = EXSA_DIR_LED3_INTENSITE;

    // LEDs OFF au démarrage
    for (uint8_t i = 0; i < 4; i++) {
        pca->setPWM(C[i], 0, 0);
    }
}

/*-------------------------------------------------------------
   setDirection(direction)
   -------------------------------------------------------------
   direction ∈ [0..4]
     - 0 → toutes LED OFF
     - 1 → LED 0 ON
     - 2 → LED 0..1 ON
     - 3 → LED 0..2 ON
     - 4 → LED 0..3 ON
--------------------------------------------------------------*/
bool EXSA_Direction::setDirection(uint8_t direction) noexcept
{
    if (direction > 4)
        direction = 4;

    bool changed = false;

    for (uint8_t i = 0; i < 4; i++) {

        const bool on = (i < direction);

        if (etats[i] != on)
            changed = true;

        etats[i] = on;

        // Conversion intensité 0–255 → PWM 0–4095
        const uint16_t pwm = on ? static_cast<uint16_t>(intensites[i]) * 16u : 0u;

        pca->setPWM(C[i], 0, pwm);
    }

    return changed;
}

/*-------------------------------------------------------------
   setIntensity(index, intensite)
   -------------------------------------------------------------
   Change l’intensité d’une LED directionnelle.
   Si la LED est ON, la nouvelle intensité est appliquée immédiatement.
--------------------------------------------------------------*/
void EXSA_Direction::setIntensity(uint8_t index, uint8_t intensite) noexcept
{
    if (index >= 4)
        return;

    intensites[index] = intensite;

    if (etats[index]) {
        const uint16_t pwm = static_cast<uint16_t>(intensite) * 16u;
        pca->setPWM(C[index], 0, pwm);
    }
}

/* ============================================================
   Fin de EXSA_Direction.cpp
   ============================================================ */
