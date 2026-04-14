#include <Arduino.h>
#include "EXSA_Main.h"
#include "EXSA_Config.h"
#include "esp_task_wdt.h"

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

    // Ajout de la tâche au watchdog
    esp_task_wdt_add(nullptr);

    for (;;)
    {
        // Reset watchdog
        esp_task_wdt_reset();

        // Boucle principale EXSA
        EXSA_Main::loop();

        // Yield FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ------------------------------------------------------------
// Setup : création de la tâche EXSA
// ------------------------------------------------------------
void setup()
{
#if EXSA_DEBUG
    Serial.begin(EXSA_UART_BAUDRATE);
    delay(50);
    Serial.println("\n[BOOT] EXSA démarrage (RTOS + WDT)...");
#endif

    // Initialisation EXSA
    EXSA_Main::begin();

    // Initialisation watchdog
    esp_task_wdt_init(EXSA_WDT_TIMEOUT_SEC, true);

    // Création de la tâche EXSA
    xTaskCreatePinnedToCore(
        exsaTask,
        "EXSA_Task",
        4096,
        nullptr,
        1,              // priorité basse mais suffisante
        &exsaTaskHandle,
        APP_CPU_NUM     // cœur 1 (évite conflit WiFi)
    );
}

// ------------------------------------------------------------
// Loop Arduino : inutilisée
// ------------------------------------------------------------
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
