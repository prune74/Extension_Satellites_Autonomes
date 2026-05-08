#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_BoosterHw.h
 * ------------------------------------------------------------
 *  Couche matérielle du Booster Discovery 2026.
 *
 *  Ce module encapsule TOUT le hardware voie :
 *
 *    - DRV8874 (pont en H)
 *         • enableOutput()  → active la voie (nSLEEP + PWM)
 *         • disableOutput() → coupe la voie
 *         • isFaultActive() → lit la broche nFAULT
 *
 *    - PWM DCC (pilotage voie)
 *         • setupPwmDcc()   → configuration LEDC
 *         • applyDcc()      → applique le bit DCC reçu via CAN
 *
 *    - Cutout RailCom
 *         • enableCutout()  → coupe PWM (EN = 0)
 *         • disableCutout() → réactive la voie (EN = 255)
 *
 *    - ADC (télémétrie + RailCom HF)
 *         • readCurrent_mA()     → courant voie via IPROPI (DRV8874)
 *         • readVoltage_mV()     → tension voie (diviseur 10k/47k)
 *         • readRailcomAdcRaw()  → lecture ADC1 directe (HF)
 *
 *  Cette couche NE CONTIENT AUCUNE LOGIQUE :
 *  elle exécute uniquement des actions matérielles.
 *
 *  Toute la logique (sécurité, cutout, RailCom, CAN)
 *  est gérée dans EXSA_Booster.cpp.
 * ============================================================
 */

class EXSA_BoosterHw
{
public:
    /*
     * --------------------------------------------------------
     * begin()
     * --------------------------------------------------------
     * Initialisation complète du hardware voie :
     *   - DRV8874 (pont en H)
     *   - PWM DCC (LEDC)
     *   - ADC (courant / tension / RailCom)
     *
     * Appelé une seule fois dans EXSA_Booster::begin().
     */
    static void begin();

    /*
     * --------------------------------------------------------
     * applyDcc()
     * --------------------------------------------------------
     * Applique le bit DCC reçu via CAN (0x100).
     * data[0] = bit DCC (0 ou 1)
     * len     = 1
     *
     * PHASE = bit DCC
     * PWM   = amplitude (toujours 255 hors cutout)
     */
    static void applyDcc(const uint8_t *data, uint8_t len);

    /*
     * --------------------------------------------------------
     * Cutout RailCom
     * --------------------------------------------------------
     * enableCutout() :
     *    - coupe PWM (EN = 0)
     *
     * disableCutout() :
     *    - réactive le PWM (EN = 255)
     */
    static void enableCutout();
    static void disableCutout();

    /*
     * --------------------------------------------------------
     * DRV8874 — pont en H
     * --------------------------------------------------------
     * enableOutput()  → s’assure que nSLEEP est actif
     * disableOutput() → met nSLEEP à LOW + PWM OFF
     * isFaultActive() → lit la broche nFAULT (LOW = défaut)
     */
    static void enableOutput();
    static void disableOutput();
    static bool  isFaultActive();

    /*
     * --------------------------------------------------------
     * Télémétrie voie
     * --------------------------------------------------------
     * readCurrent_mA() :
     *    - lit IPROPI (ADC)
     *    - convertit en mA via R_IPROPI et A_IPROPI
     *
     * readVoltage_mV() :
     *    - lit le diviseur 10k / 47k
     *    - convertit en mV
     */
    static uint16_t readCurrent_mA();
    static uint16_t readVoltage_mV();

    /*
     * --------------------------------------------------------
     * RailCom — lecture ADC haute fréquence
     * --------------------------------------------------------
     * readRailcomAdcRaw() :
     *    - lecture directe ADC1 (sans filtrage)
     *    - utilisée par EXSA_BoosterRailCom pour décodage HF
     */
    static int16_t readRailcomAdcRaw();

private:
    /*
     * --------------------------------------------------------
     * Sous-systèmes internes
     * --------------------------------------------------------
     * setupPwmDcc()   → configuration LEDC (20 kHz)
     * setupDrv8874()  → configuration broches nSLEEP/PH/nFAULT
     * setupAdc()      → configuration ADC1 (RailCom + télémétrie)
     */
    static void setupPwmDcc();
    static void setupDrv8874();
    static void setupAdc();
};
