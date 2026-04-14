#include "EXSA_Quadrature.h"
#include "EXSA_Config.h"
#include "EXSA_Essieux.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Queue FreeRTOS pour transmettre les événements A/B
static QueueHandle_t quadQueue = nullptr;

// -----------------------------------------------------------------------------
// ISR : capture brute
// -----------------------------------------------------------------------------
void IRAM_ATTR isrQuadrature()
{
    uint8_t a = digitalRead(EXSA_QUAD_A_PIN);
    uint8_t b = digitalRead(EXSA_QUAD_B_PIN);
    uint8_t etat = (a << 1) | b;

    // Envoi dans la queue (non bloquant)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(quadQueue, &etat, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
}

// -----------------------------------------------------------------------------
// initQueue()
// -----------------------------------------------------------------------------
void EXSA_Quadrature::initQueue() noexcept
{
    quadQueue = xQueueCreate(32, sizeof(uint8_t));
}

// -----------------------------------------------------------------------------
// lireEvenement()
// -----------------------------------------------------------------------------
bool EXSA_Quadrature::lireEvenement(uint8_t &etat) noexcept
{
    return xQueueReceive(quadQueue, &etat, 0) == pdTRUE;
}

// -----------------------------------------------------------------------------
// installerInterruptions()
// -----------------------------------------------------------------------------
void EXSA_Quadrature::installerInterruptions() noexcept
{
    pinMode(EXSA_QUAD_A_PIN, INPUT);
    pinMode(EXSA_QUAD_B_PIN, INPUT);

    // État initial (non utilisé ici, mais utile si tu veux l’exposer plus tard)
    uint8_t etatInitial =
        (digitalRead(EXSA_QUAD_A_PIN) << 1) |
         digitalRead(EXSA_QUAD_B_PIN);

    (void)etatInitial;

    attachInterrupt(EXSA_QUAD_A_PIN, isrQuadrature, CHANGE);
    attachInterrupt(EXSA_QUAD_B_PIN, isrQuadrature, CHANGE);
}
