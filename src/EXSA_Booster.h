#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_Booster.h
 * ------------------------------------------------------------
 *  Interface du Booster local Discovery 2026.
 *
 *  Ce module est le "répéteur DCC intelligent" :
 *    - applique le bit DCC reçu via CAN (LaBox = maître DCC)
 *    - gère le cutout RailCom (fenêtre de mesure)
 *    - déclenche le décodage RailCom (adresse courte/longue)
 *    - mesure courant/tension (télémétrie CAN 0x102)
 *    - publie les adresses RailCom (CAN 0x103)
 *    - applique les règles de sécurité Discovery 2026 :
 *         • FAULT DRV8801
 *         • court-circuit local
 *         • tension anormale
 *         • défauts des autres boosters (CAN)
 *         • inversion de phase
 *         • cutout global
 *
 *  Appelé exclusivement depuis BoosterCore_Task (RTOS),
 *  à une cadence fixe de 1 ms.
 *
 *  Le .cpp contient toute la logique :
 *    - updateCutout()
 *    - applyDccFrame()
 *    - updateTelemetry()
 *    - updateRailcom()
 *    - checkSafety()
 *
 *  Le header expose uniquement l’API publique.
 * ============================================================
 */

class EXSA_Booster
{
public:
    /*
     * --------------------------------------------------------
     * begin()
     * --------------------------------------------------------
     * Initialisation complète du booster :
     *   - DRV8801 (H-bridge)
     *   - PWM DCC
     *   - ADC courant / tension
     *   - ADC RailCom HF + timer ISR
     *
     * Appelé une seule fois dans BoosterCore_Task.
     */
    static void begin();

    /*
     * --------------------------------------------------------
     * update()
     * --------------------------------------------------------
     * Boucle principale du booster (1 ms).
     * Ordre strict :
     *   1) updateCutout()
     *   2) applyDccFrame()
     *   3) updateTelemetry()
     *   4) RailCom::update()
     *   5) updateRailcom()
     *   6) checkSafety()
     *
     * Appelé exclusivement depuis BoosterCore_Task.
     */
    static void update();

private:
    /*
     * --------------------------------------------------------
     * Sous-fonctions internes (appelées dans update())
     * --------------------------------------------------------
     */

    // Applique le bit DCC reçu via CAN (ID 0x100)
    static void applyDccFrame();

    // Active/désactive la fenêtre RailCom (ID 0x101)
    static void updateCutout();

    // Mesure courant/tension + envoi CAN 0x102
    static void updateTelemetry();

    // Publication des adresses RailCom (CAN 0x103)
    static void updateRailcom();

    // Sécurité voie (locale + globale Discovery 2026)
    static void checkSafety();

    /*
     * --------------------------------------------------------
     * Timers internes (ms)
     * --------------------------------------------------------
     * _lastTelemetryTime :
     *    cadence 20 ms pour la télémétrie CAN 0x102
     *
     * _lastRailcomTime :
     *    cadence 10 ms pour la publication RailCom 0x103
     */
    static uint32_t _lastTelemetryTime;
    static uint32_t _lastRailcomTime;
};
