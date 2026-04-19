#pragma once
#include <Arduino.h>

/* ============================================================
   EXSA_Pins.h
   ------------------------------------------------------------
   Définition des broches physiques utilisées par l’EXSA.
   Version MCP23017 + Booster Discovery 2026 (CAN natif ESP32)
   ============================================================ */

/* -----------------------------
   UART EXSA ↔ SA
------------------------------*/
#define EXSA_UART_BAUDRATE 115200
#define EXSA_UART_RX_PIN   25
#define EXSA_UART_TX_PIN   26

/* -----------------------------
   Capteurs quadrature (A/B)
------------------------------*/
#define EXSA_QUAD_A_PIN    34
#define EXSA_QUAD_B_PIN    35

/* -----------------------------
   Capteur de présence
------------------------------*/
#define EXSA_PRESENCE_PIN  15

/* -----------------------------
   Sélecteurs physiques H/AH + Booster
------------------------------*/
#define EXSA_DIP_HAH_PIN       12   // DIP 1 = H / AH
#define EXSA_DIP_BOOSTER_PIN    2   // DIP 2 = Booster ON/OFF (boot-safe)

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
   ============================================================ */
#define EXSA_MCP23017_ADDR 0x20

#define EXSA_SW0_DROIT   0
#define EXSA_SW0_DEVIE   1
#define EXSA_SW1_DROIT   2
#define EXSA_SW1_DEVIE   3
#define EXSA_SW2_DROIT   4
#define EXSA_SW2_DEVIE   5

#define EXSA_MCP23017_INT_PIN  17   // GPIO17 → interruption propre

/* ============================================================
   Section Booster Discovery 2026
   ------------------------------------------------------------
   DRV8801 + PWM DCC + ADC RailCom + CAN Booster (natif)
   ============================================================ */

/* --- DRV8801 --- */
#define EXSA_DRV_ENABLE      19
#define EXSA_DRV_PHASE       21
#define EXSA_DRV_FAULT       22

/* --- PWM DCC --- */
#define EXSA_DCC_PIN         23   // PWM LEDC canal 0

/* --- ADC Télémétrie --- */
#define EXSA_ADC_CURRENT     32   // ADC1_CH4 → courant voie
#define EXSA_ADC_VOLTAGE     33   // ADC1_CH5 → tension voie

/* --- ADC RailCom HF (dédié) --- */
#define EXSA_ADC_RAILCOM     36   // ADC1_CH0 → RailCom HF (rapide)

/* --- CAN Booster (natif ESP32) --- */
#define EXSA_CAN_RX          4
#define EXSA_CAN_TX          5

/* --- LED d’erreur Booster --- */
#define EXSA_LED_ERROR_PIN   16   // LED rouge (boot-safe)
