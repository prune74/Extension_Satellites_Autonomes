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
 *    - DRV8801 (pont en H)
 *         • enableOutput()  → active la voie
 *         • disableOutput() → coupe la voie
 *         • isFaultActive() → lit la broche FAULT
 *
 *    - PWM DCC (pilotage voie)
 *         • setupPwmDcc()   → configuration LEDC
 *         • applyDcc()      → applique le bit DCC reçu via CAN
 *
 *    - Cutout RailCom
 *         • enableCutout()  → coupe PWM + pont H
 *         • disableCutout() → réactive la voie
 *
 *    - ADC (télémétrie + RailCom HF)
 *         • readCurrent_mA()     → courant voie (shunt 0.14 Ω)
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
     *   - DRV8801 (pont en H)
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
     * PWM   = amplitude (toujours 255)
     */
    static void applyDcc(const uint8_t *data, uint8_t len);

    /*
     * --------------------------------------------------------
     * Cutout RailCom
     * --------------------------------------------------------
     * enableCutout() :
     *    - coupe PWM
     *    - désactive le pont H
     *
     * disableCutout() :
     *    - réactive le pont H
     *    - PWM réactivé par applyDcc()
     */
    static void enableCutout();
    static void disableCutout();

    /*
     * --------------------------------------------------------
     * DRV8801 — pont en H
     * --------------------------------------------------------
     * enableOutput()  → active la voie
     * disableOutput() → coupe la voie
     * isFaultActive() → lit la broche FAULT (LOW = défaut)
     */
    static void enableOutput();
    static void disableOutput();
    static bool  isFaultActive();

    /*
     * --------------------------------------------------------
     * Télémétrie voie
     * --------------------------------------------------------
     * readCurrent_mA() :
     *    - lit le shunt 0.14 Ω
     *    - convertit en mA
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
     * setupDrv8801()  → configuration broches ENABLE/PHASE/FAULT
     * setupAdc()      → configuration ADC1 (RailCom + télémétrie)
     */
    static void setupPwmDcc();
    static void setupDrv8801();
    static void setupAdc();
};
