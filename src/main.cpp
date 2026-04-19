#include <Arduino.h>
#include "EXSA_Main.h"
#include "EXSA_Config.h"
#include "esp_task_wdt.h"

static TaskHandle_t exsaTaskHandle = nullptr;

// Timeout watchdog (en secondes)
static constexpr int EXSA_WDT_TIMEOUT_SEC = 2;

// ------------------------------------------------------------
// Tâche FreeRTOS dédiée à EXSA (logique principale)
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
// Setup : création de la seule tâche EXSA
// Le Booster sera lancé par EXSA_Main::begin() si DIP = ON
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
}

// ------------------------------------------------------------
// Loop Arduino : inutilisée
// ------------------------------------------------------------
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
