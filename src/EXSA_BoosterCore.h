#pragma once

/*
 * ============================================================
 *  EXSA_BoosterCore — Tâche FreeRTOS Booster Discovery 2026
 * ------------------------------------------------------------
 *  - Initialise :
 *        • CAN Booster
 *        • Booster HW (DCC + DRV8801 + ADC)
 *        • RailCom
 *
 *  - Boucle temps réel 1 ms :
 *        • EXSA_CanBooster::process()
 *        • EXSA_Booster::update()
 *
 *  - Lancement depuis EXSA_Main::begin()
 * ============================================================
 */

class EXSA_BoosterCore
{
public:
    static void startTask();   // idempotent

private:
    static void taskEntry(void *param);
};
