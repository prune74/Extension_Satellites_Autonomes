#include "EXSA_BoosterCore.h"
#include "EXSA_CanBooster.h"
#include "EXSA_Booster.h"
#include "EXSA_BoosterRailCom.h"   // <-- AJOUT RailCom
#include "EXSA_Config.h"

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static TaskHandle_t s_boosterTaskHandle = nullptr;

void EXSA_BoosterCore::startTask()
{
    if (s_boosterTaskHandle != nullptr)
        return; // déjà créée

    BaseType_t res = xTaskCreatePinnedToCore(
        EXSA_BoosterCore::taskEntry,
        "BoosterCore",
        4096,
        nullptr,
        3,              // priorité haute
        &s_boosterTaskHandle,
        APP_CPU_NUM
    );

#if EXSA_DEBUG
    if (res == pdPASS)
        Serial.println("[RTOS] Tâche BoosterCore créée");
    else
        Serial.println("[RTOS] ERREUR création tâche BoosterCore");
#endif
}

void EXSA_BoosterCore::taskEntry(void *param)
{
    (void)param;

#if EXSA_DEBUG
    Serial.println("[RTOS] BoosterCore : initialisation...");
#endif

    EXSA_CanBooster::begin();
    EXSA_Booster::begin();

    esp_task_wdt_add(nullptr);

#if EXSA_DEBUG
    Serial.println("[RTOS] BoosterCore : OK, boucle 1 ms");
#endif

    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1);

    static bool prevCutout = false;

    for (;;)
    {
        esp_task_wdt_reset();

        // 1) CAN → met à jour cutoutActive, dccBuffer, etc.
        EXSA_CanBooster::process();

        // 2) Booster physique (DCC, DRV8801, protections…)
        EXSA_Booster::update();

        // 3) RailCom : détection cutout START / END
        bool cutout = EXSA_CanBooster::cutoutActive;

        // CUTOUT START
        if (cutout && !prevCutout) {
            EXSA_BoosterRailCom::onCutoutStart();
        }

        // CUTOUT END
        if (!cutout && prevCutout) {
            EXSA_BoosterRailCom::onCutoutEnd();

            uint16_t addr = EXSA_BoosterRailCom::getLastAddress();
            if (addr != 0) {
                EXSA_CanBooster::sendRailcomAddress(addr);
                EXSA_BoosterRailCom::clearLastAddress();
            }
        }

        prevCutout = cutout;

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
