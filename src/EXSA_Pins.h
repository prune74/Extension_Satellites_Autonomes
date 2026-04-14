#pragma once
#include <Arduino.h>

/* ============================================================
   EXSA_Pins.h
   ------------------------------------------------------------
   Définition des broches physiques utilisées par l’EXSA.
   Version MCP23017 (entrées micro-switchs déportées en I²C)
   ============================================================ */

/* -----------------------------
   UART EXSA ↔ SA
------------------------------*/
#define EXSA_UART_BAUDRATE 115200
#define EXSA_UART_RX_PIN 25
#define EXSA_UART_TX_PIN 26

/* -----------------------------
   Capteurs quadrature (A/B)
------------------------------*/
#define EXSA_QUAD_A_PIN 34
#define EXSA_QUAD_B_PIN 35

/* -----------------------------
   Capteur de présence
------------------------------*/
#define EXSA_PRESENCE_PIN 15

/* -----------------------------
   Sélecteur physique H / AH
------------------------------*/
#define EXSA_DIP_PIN 12

/* -----------------------------
   Charlieplexing (mât SNCF)
------------------------------*/
#define EXSA_MUX_P1  27
#define EXSA_MUX_P2  13
#define EXSA_MUX_P3  14
#define EXSA_MUX_P4  18

/* -----------------------------
   PCA9685 — Feux directionnels
------------------------------*/
#define EXSA_DIR_1 0
#define EXSA_DIR_2 1
#define EXSA_DIR_3 2
#define EXSA_DIR_4 3

/* -----------------------------
   PCA9685 — LED canton
------------------------------*/
#define EXSA_CANTON_OCCUPE_CH     8
#define EXSA_CANTON_LIBRE_CH      9
#define EXSA_CANTON_MOUVEMENT_CH  10
#define EXSA_CANTON_ERREUR_CH     11

/* -----------------------------
   PCA9685 — Servos (réservés)
------------------------------*/
#define EXSA_SERVO_1 12
#define EXSA_SERVO_2 13
#define EXSA_SERVO_3 14

/* ============================================================
   MCP23017 — Micro-switchs position réelle aiguilles
   ------------------------------------------------------------
   Le MCP23017 fournit 16 entrées :
   GPA0..GPA7  → Port A
   GPB0..GPB7  → Port B
   ============================================================ */

/* Adresse I²C du MCP23017 (A0/A1/A2 = GND) */
#define EXSA_MCP23017_ADDR 0x20

/* Aiguille 0 */
#define EXSA_SW0_DROIT   0   // GPA0
#define EXSA_SW0_DEVIE   1   // GPA1

/* Aiguille 1 */
#define EXSA_SW1_DROIT   2   // GPA2
#define EXSA_SW1_DEVIE   3   // GPA3

/* Aiguille 2 */
#define EXSA_SW2_DROIT   4   // GPA4
#define EXSA_SW2_DEVIE   5   // GPA5

/* Optionnel : broche d'interruption du MCP23017 */
#define EXSA_MCP23017_INT_PIN  4   // GPIO4 (au choix)
