#pragma once
#include <Arduino.h>
#include "EXSA_Pins.h"          // Broches physiques

/*
 * ============================================================
 *  EXSA_Config.h — Configuration globale EXSA (Option A)
 * ------------------------------------------------------------
 *  Ce fichier regroupe toutes les constantes de configuration :
 *    - options générales
 *    - temporisations
 *    - intensités par LED
 *    - paramètres du multiplexeur
 *
 *  Aucun aspect SNCF ici.
 *  Aucun protocole ici.
 *  Aucun bit ici.
 *
 *  Les aspects SNCF sont gérés via enum ExsaAspect
 *  dans EXSA_Signaux.h / EXSA_Signaux.cpp.
 * ============================================================
 */

/* ============================================================
 *  Options générales EXSA
 * ============================================================ */
#define EXSA_DEBUG              false
#define EXSA_DEBOUNCE_MS        5
#define EXSA_MAX_ESSIEUX        200

/* ============================================================
 *  Type de signal (mât SNCF)
 * ------------------------------------------------------------
 *  false = signal principal (rouge)
 *  true  = signal de manœuvre (violet → carré violet)
 * ============================================================ */
#define EXSA_SIGNAL_EST_MANOEUVRE   false

/* ============================================================
 *  Temporisations EXSA_Canton
 * ============================================================ */
#define EXSA_CANTON_ANIM_STEP_MS    120
#define EXSA_CANTON_MOUVEMENT_MS    200

/* ============================================================
 *  Multiplexage Charlieplexing (mât SNCF)
 * ============================================================ */
#define EXSA_MUX_PERIOD_MS          1       // Scan LED → 1 kHz
#define EXSA_MUX_BLINK_MS           500     // Clignotement 2 Hz

/* ============================================================
 *  Intensités par défaut des LED du mât SNCF
 * ------------------------------------------------------------
 *  Valeurs PWM (0–255) utilisées pour chaque LED du mât.
 * ============================================================ */
#define INTENSITE_DEFAUT_LED_RALENTISSEMENT   220
#define INTENSITE_DEFAUT_LED_RAPPEL           200
#define INTENSITE_DEFAUT_LED_SEMAPHORE        255
#define INTENSITE_DEFAUT_LED_LIBRE            180
#define INTENSITE_DEFAUT_LED_AVERTISSEMENT    255
#define INTENSITE_DEFAUT_LED_OEILLETON        80
#define INTENSITE_DEFAUT_LED_MANOEUVRE        255
#define INTENSITE_DEFAUT_LED_CARRE            255
#define INTENSITE_DEFAUT_LED_CARRE_VIOLET     255

/* ============================================================
 *  Intensité des LED directionnelles (0–255)
 * ============================================================ */
#define EXSA_DIR_LED0_INTENSITE   255
#define EXSA_DIR_LED1_INTENSITE   255
#define EXSA_DIR_LED2_INTENSITE   255
#define EXSA_DIR_LED3_INTENSITE   255

/* ============================================================
 *  Sécurité Booster Discovery 2026
 * ------------------------------------------------------------
 *  Seuils configurables pour la protection voie :
 *
 *  - EXSA_BOOSTER_MAX_COURANT_mA
 *      Courant max avant coupure (court-circuit local)
 *
 *  - EXSA_BOOSTER_MIN_TENSION_mV
 *      Tension min avant coupure (voie OFF / alim faible)
 *
 *  - EXSA_BOOSTER_PHASE_TOLERANCE
 *      Tolérance d’inversion de phase (0 = strict)
 *
 *  - EXSA_BOOSTER_ENABLE_GLOBAL_PROTECTION
 *      true  = coupe si un autre booster signale un défaut
 *      false = ignore les défauts des autres boosters
 *
 *  - EXSA_BOOSTER_ENABLE_GLOBAL_CUTOUT
 *      true  = coupe si cutout global actif
 *      false = ignore le cutout global
 *
 * ============================================================ */

#define EXSA_BOOSTER_MAX_COURANT_mA        1400   // Limite soft pour booster 1,5 A
#define EXSA_BOOSTER_MIN_TENSION_mV        8000   // 8 V
#define EXSA_BOOSTER_PHASE_TOLERANCE       0
#define EXSA_BOOSTER_ENABLE_GLOBAL_PROTECTION  true
#define EXSA_BOOSTER_ENABLE_GLOBAL_CUTOUT      true

/* ============================================================
 *  Mesure courant via DRV8874 (IPROPI)
 * ------------------------------------------------------------
 *  - EXSA_IPROPI_R_OHMS : résistance entre IPROPI et GND
 *  - EXSA_IPROPI_GAIN_A_PER_A : gain interne (A_IPROPI)
 *
 *  Formule :
 *      I = V_IPROPI / (R_IPROPI * A_IPROPI)
 *
 *  Avec :
 *      R_IPROPI = 3,6 kΩ → I_TRIP ≈ 1,5 A
 *      A_IPROPI = 455 µA/A
 * ============================================================ */

#define EXSA_IPROPI_R_OHMS          3600.0f     // 3,6 kΩ
#define EXSA_IPROPI_GAIN_A_PER_A    0.000455f   // 455 µA/A

/* ============================================================
 *  Aucun aspect ici !
 * ------------------------------------------------------------
 *  Les aspects SNCF sont définis dans :
 *      → EXSA_Signaux.h (enum ExsaAspect)
 *
 *  Le protocole SA → EXSA utilise un octet (enum simple).
 * ============================================================ */
