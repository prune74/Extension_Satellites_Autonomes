#include <Arduino.h>
#include "EXSA_Main.h"
#include "EXSA_Config.h"
#include "esp_task_wdt.h"
#include "EXSA_CanBooster.h"
#include "EXSA_Booster.h"

static TaskHandle_t exsaTaskHandle = nullptr;

// Timeout watchdog (en secondes)
static constexpr int EXSA_WDT_TIMEOUT_SEC = 2;

// ------------------------------------------------------------
// Tâche FreeRTOS dédiée à EXSA
// ------------------------------------------------------------
void exsaTask(void* parameter)
{
#if EXSA_DEBUG
    Serial.println("[RTOS] Tâche EXSA démarrée");
#endif

    esp_task_wdt_add(nullptr);

    for (;;)
    {
        esp_task_wdt_reset();
        EXSA_Main::loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ------------------------------------------------------------
// Tâche FreeRTOS dédiée au CAN Booster
// ------------------------------------------------------------
void BoosterCAN_Task(void* parameter)
{
#if EXSA_DEBUG
    Serial.println("[RTOS] Tâche CAN Booster démarrée");
#endif

    EXSA_CanBooster::begin();

    for (;;)
    {
        EXSA_CanBooster::process();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ------------------------------------------------------------
// Tâche FreeRTOS dédiée au Booster (DCC + DRV8801 + RailCom)
// ------------------------------------------------------------
void BoosterCore_Task(void* parameter)
{
#if EXSA_DEBUG
    Serial.println("[RTOS] Tâche BoosterCore démarrée");
#endif

    EXSA_Booster::begin();
    esp_task_wdt_add(nullptr);

    for (;;)
    {
        esp_task_wdt_reset();
        EXSA_Booster::update();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1 ms
    }
}

// ------------------------------------------------------------
// Setup : création des tâches EXSA + CAN Booster + BoosterCore
// ------------------------------------------------------------
void setup()
{
#if EXSA_DEBUG
    Serial.begin(EXSA_UART_BAUDRATE);
    delay(50);
    Serial.println("\n[BOOT] EXSA démarrage (RTOS + WDT)...");
#endif

    EXSA_Main::begin();

    esp_task_wdt_init(EXSA_WDT_TIMEOUT_SEC, true);

    // Tâche EXSA
    xTaskCreatePinnedToCore(
        exsaTask,
        "EXSA_Task",
        4096,
        nullptr,
        1,
        &exsaTaskHandle,
        APP_CPU_NUM
    );

    // Tâche CAN Booster
    xTaskCreatePinnedToCore(
        BoosterCAN_Task,
        "BoosterCAN_Task",
        4096,
        nullptr,
        2,
        nullptr,
        APP_CPU_NUM
    );

    // Tâche BoosterCore (DCC + DRV8801 + RailCom)
    xTaskCreatePinnedToCore(
        BoosterCore_Task,
        "BoosterCore_Task",
        4096,
        nullptr,
        3,              // priorité la plus haute
        nullptr,
        APP_CPU_NUM
    );
}

// ------------------------------------------------------------
// Loop Arduino : inutilisée
// ------------------------------------------------------------
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
