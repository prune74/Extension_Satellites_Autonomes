/*
 * EXSA_Direction.h
 *
 * Gestion des feux directionnels (barrettes cumulatives) via PCA9685.
 *
 * Un EXSA universel dispose d’un bloc unique de 4 LED directionnelles.
 * Le SA envoie une valeur entre 0 et 4 :
 *
 *   0 → aucune LED
 *   1 → LED1
 *   2 → LED1 + LED2
 *   3 → LED1 + LED2 + LED3
 *   4 → LED1 + LED2 + LED3 + LED4
 *
 * Chaque LED possède :
 *   - un état ON/OFF
 *   - une intensité réglable (0–255)
 *
 * Le PCA9685 permet un contrôle PWM précis (0–4095).
 *
 * EXSA_Main appelle setDirection() lorsqu’il reçoit E8/E9.
 */

#pragma once
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

class EXSA_Direction {
public:

    /*
     * Constructeur
     * ---------------------------------------------------------
     * driver : pointeur vers le PCA9685 partagé
     * c1..c4 : numéros de canaux PCA pour les 4 LED directionnelles
     *
     * Chaque LED directionnelle correspond à un canal PCA9685.
     */
    explicit EXSA_Direction(Adafruit_PWMServoDriver* driver,
                            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) noexcept;

    /*
     * setDirection(direction)
     * ---------------------------------------------------------
     * Définit la direction affichée (0 à 4).
     *
     * Les LED sont cumulatives :
     *   direction = 3 → LED1 + LED2 + LED3
     *
     * Retourne true si la direction affichée a changé.
     */
    [[nodiscard]] bool setDirection(uint8_t direction) noexcept;

    /*
     * setIntensity(index, intensite)
     * ---------------------------------------------------------
     * Change l’intensité d’une LED (0–255).
     * index = 0..3
     *
     * Si la LED est allumée, le PWM est mis à jour immédiatement.
     * Sinon, la nouvelle intensité sera appliquée lors du prochain ON.
     */
    void setIntensity(uint8_t index, uint8_t intensite) noexcept;

private:

    Adafruit_PWMServoDriver* pca;  // Pointeur vers le PCA9685 partagé

    uint8_t C[4];                  // Numéros de canaux PCA pour les 4 LED
    uint8_t intensites[4];         // Intensité 0–255 pour chaque LED
    bool etats[4];                 // État ON/OFF de chaque LED
};
/* ============================================================
   Fin de EXSA_Direction.h
   ============================================================ */