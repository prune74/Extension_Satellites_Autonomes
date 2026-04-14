#include "EXSA_Servo.h"
#include "EXSA_Pins.h"
#include "EXSA_Config.h"

/*
 * ============================================================
 *  EXSA_Servo.cpp
 * ------------------------------------------------------------
 *  Implémentation du module de gestion des servos EXSA.
 *
 *  - Conversion µs → PWM pour PCA9685
 *  - Interpolation progressive
 *  - Gestion de 3 servos maximum
 *  - Compatible avec les trames F0/F1/F2 envoyées par SA
 *
 *  Le PCA9685 est déclaré dans EXSA_Main :
 *      Adafruit_PWMServoDriver g_pca9685;
 * ============================================================
 */

extern Adafruit_PWMServoDriver g_pca9685;

namespace
{
    /* ------------------------------------------------------------
       Nombre maximum de servos gérés par EXSA
       ------------------------------------------------------------ */
    constexpr uint8_t SERVO_COUNT = 3;

    /* ------------------------------------------------------------
       Mapping index logique → canal PCA9685
       (défini dans EXSA_Pins.h)
       ------------------------------------------------------------ */
    constexpr uint8_t SERVO_CHANNEL[SERVO_COUNT] = {
        EXSA_SERVO_1,
        EXSA_SERVO_2,
        EXSA_SERVO_3
    };

    /* ------------------------------------------------------------
       Limites mécaniques envoyées par SA (µs)
       ------------------------------------------------------------ */
    constexpr uint16_t SERVO_US_MIN   = 800;
    constexpr uint16_t SERVO_US_MAX   = 2400;
    constexpr uint16_t SERVO_US_RANGE = SERVO_US_MAX - SERVO_US_MIN;

    /* ------------------------------------------------------------
       Conversion microsecondes → PWM PCA9685 (0–4095)
       Version améliorée : clamp + lisibilité
       ------------------------------------------------------------ */
    inline uint16_t usToPwm(uint16_t us)
    {
        if (us < SERVO_US_MIN) us = SERVO_US_MIN;
        else if (us > SERVO_US_MAX) us = SERVO_US_MAX;

        return (uint32_t)(us - SERVO_US_MIN) * 4095u / SERVO_US_RANGE;
    }

    /* ------------------------------------------------------------
       Structure interne d’un servo
       ------------------------------------------------------------ */
    struct ServoData {
        uint16_t posDroit_pwm;   // position droite convertie en PWM
        uint16_t posDevie_pwm;   // position déviée convertie en PWM
        uint16_t speed_pwm;      // vitesse (pas PWM par update)
        uint16_t current_pwm;    // position actuelle
        uint16_t target_pwm;     // position cible
    };

    ServoData servos[SERVO_COUNT];

    inline bool indexValide(uint8_t index)
    {
        return index < SERVO_COUNT;
    }

    inline void appliquerPWM(uint8_t index)
    {
        g_pca9685.setPWM(SERVO_CHANNEL[index], 0, servos[index].current_pwm);
    }
}

/* ============================================================
   Initialisation
   ============================================================ */
void EXSA_Servo::begin() noexcept
{
    for (uint8_t i = 0; i < SERVO_COUNT; ++i)
    {
        // Valeurs par défaut (1500 µs)
        servos[i].posDroit_pwm = usToPwm(1500);
        servos[i].posDevie_pwm = usToPwm(1500);
        servos[i].speed_pwm    = 10;

        servos[i].current_pwm  = servos[i].posDroit_pwm;
        servos[i].target_pwm   = servos[i].posDroit_pwm;

        appliquerPWM(i);
    }
}

/* ============================================================
   Interpolation progressive
   ============================================================ */
void EXSA_Servo::update() noexcept
{
    for (uint8_t i = 0; i < SERVO_COUNT; ++i)
    {
        uint16_t cur = servos[i].current_pwm;
        uint16_t tgt = servos[i].target_pwm;

        if (cur == tgt)
            continue; // rien à faire

        uint16_t step = servos[i].speed_pwm;
        if (step == 0)
            step = 1;

        if (cur < tgt)
        {
            uint16_t delta = tgt - cur;
            servos[i].current_pwm = (delta <= step) ? tgt : cur + step;
        }
        else
        {
            uint16_t delta = cur - tgt;
            servos[i].current_pwm = (delta <= step) ? tgt : cur - step;
        }

        appliquerPWM(i);
    }
}

/* ============================================================
   F0 — move(index, direction)
   direction : 0 = droite, 1 = déviée
   ============================================================ */
void EXSA_Servo::move(uint8_t index, uint8_t direction) noexcept
{
    if (!indexValide(index))
        return;

    servos[index].target_pwm =
        (direction == 0) ? servos[index].posDroit_pwm
                         : servos[index].posDevie_pwm;
}

/* ============================================================
   F1 — configure(index, posDroit_us, posDevie_us, speed_us)
   Version améliorée :
     - conversion µs → PWM plus claire
     - commentaire pédagogique sur la vitesse
     - protection anti‑saut brutal
   ============================================================ */
void EXSA_Servo::configure(uint8_t index,
                           uint16_t posDroit_us,
                           uint16_t posDevie_us,
                           uint16_t speed_us) noexcept
{
    if (!indexValide(index))
        return;

    // Conversion µs → PWM
    servos[index].posDroit_pwm = usToPwm(posDroit_us);
    servos[index].posDevie_pwm = usToPwm(posDevie_us);

    /*
     * Conversion vitesse µs → pas PWM :
     *   - speed_us = durée totale du mouvement (ex : 500 ms)
     *   - update() est appelé ~50 fois par seconde
     *   - on convertit donc en "pas PWM par update"
     */
    servos[index].speed_pwm =
        (uint32_t)speed_us * 4095u / SERVO_US_RANGE / 50u;

    if (servos[index].speed_pwm < 1)
        servos[index].speed_pwm = 1;

    /*
     * Protection anti‑saut brutal :
     * Si une nouvelle configuration arrive pendant un mouvement,
     * on synchronise la position actuelle sur la cible précédente.
     */
    servos[index].current_pwm = servos[index].target_pwm;
}

/* ============================================================
   F2 — test(index)
   Bascule entre droite ↔ dévié
   ============================================================ */
void EXSA_Servo::test(uint8_t index) noexcept
{
    if (!indexValide(index))
        return;

    uint16_t mid = (servos[index].posDroit_pwm + servos[index].posDevie_pwm) / 2;

    servos[index].target_pwm =
        (servos[index].current_pwm < mid)
        ? servos[index].posDevie_pwm
        : servos[index].posDroit_pwm;
}

/* ============================================================
   Accesseurs pour la sécurité (EXSA_Switches)
   ------------------------------------------------------------
   EXSA_Servo expose son état interne sans prendre de décision.
   ============================================================ */
bool EXSA_Servo::isMoving(uint8_t index) noexcept
{
    if (!indexValide(index))
        return false;

    return servos[index].current_pwm != servos[index].target_pwm;
}

uint16_t EXSA_Servo::getCurrentPwm(uint8_t index) noexcept
{
    if (!indexValide(index))
        return 0;

    return servos[index].current_pwm;
}

uint16_t EXSA_Servo::getTargetPwm(uint8_t index) noexcept
{
    if (!indexValide(index))
        return 0;

    return servos[index].target_pwm;
}

/* ============================================================
   Fin de EXSA_Servo.cpp
   ============================================================ */