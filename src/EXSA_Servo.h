#pragma once
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

/*
 * ============================================================
 *  EXSA_Servo.h — Version finale 2026
 * ------------------------------------------------------------
 *  Gestion des servos d’aiguillage pour EXSA.
 *
 *  Commandes reçues du SA :
 *    - F0 : move(index, direction)
 *    - F1 : configure(index, posDroit_us, posDevie_us, speed_us)
 *    - F2 : test(index)
 *
 *  Le module :
 *    - convertit les positions µs → PWM (0–4095)
 *    - interpole progressivement vers la cible
 *    - expose son état interne pour la sécurité aiguilles
 *
 *  EXSA_Switches utilise ces accesseurs pour :
 *    - détecter un mouvement
 *    - calculer un temps max de déplacement
 *    - détecter un blocage
 * ============================================================
 */

namespace EXSA_Servo
{
    /* ---------------------------------------------------------
       Cycle de vie
       --------------------------------------------------------- */
    void begin() noexcept;
    void update() noexcept;

    /* ---------------------------------------------------------
       Commandes SA → EXSA
       --------------------------------------------------------- */
    void move(uint8_t index, uint8_t direction) noexcept;

    void configure(uint8_t index,
                   uint16_t posDroit_us,
                   uint16_t posDevie_us,
                   uint16_t speed_us) noexcept;

    void test(uint8_t index) noexcept;

    /* ---------------------------------------------------------
       Accesseurs pour EXSA_Switches (sécurité aiguilles)
       ---------------------------------------------------------
       Ces fonctions permettent au module de sécurité :
         - de savoir si un servo est encore en mouvement
         - de connaître la position PWM actuelle
         - de connaître la position PWM cible
       --------------------------------------------------------- */

    // true si current_pwm != target_pwm
    bool isMoving(uint8_t index) noexcept;

    // position PWM actuelle
    uint16_t getCurrentPwm(uint8_t index) noexcept;

    // position PWM cible
    uint16_t getTargetPwm(uint8_t index) noexcept;
}

/* ============================================================
   Fin de EXSA_Servo.h
   ============================================================ */
