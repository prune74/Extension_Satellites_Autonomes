#include "EXSA_BoosterHw.h"
#include "EXSA_Config.h"
#include "EXSA_Pins.h"
#include <Arduino.h>
#include <driver/adc.h>

/*
 * ============================================================
 *  EXSA_BoosterHw — Couche matérielle du Booster Discovery 2026
 *  Version DRV8874 (PH/EN + IPROPI + RailCom)
 * ============================================================
 */

static constexpr int LEDC_CHANNEL = 0;

/* ============================================================
 *  begin() — Initialisation complète du hardware voie
 * ============================================================ */
void EXSA_BoosterHw::begin()
{
    setupDrv8874();   // pont en H DRV8874
    setupPwmDcc();    // PWM DCC (EN)
    setupAdc();       // ADC courant / tension / RailCom
}

/* ============================================================
 *  DRV8874 — Gestion du pont en H
 * ============================================================ */

void EXSA_BoosterHw::setupDrv8874()
{
    pinMode(EXSA_DRV_NSLEEP, OUTPUT);
    pinMode(EXSA_DRV_PHASE,  OUTPUT);
    pinMode(EXSA_DRV_FAULT,  INPUT_PULLUP);

    // PMODE et IMODE câblés en dur à GND sur le PCB
    // (PH/EN mode + Quad-Level 1)

    digitalWrite(EXSA_DRV_NSLEEP, LOW);   // sécurité au démarrage
    digitalWrite(EXSA_DRV_PHASE,  LOW);
    delay(5);
    digitalWrite(EXSA_DRV_NSLEEP, HIGH);  // activation DRV8874
}

void EXSA_BoosterHw::enableOutput()
{
    // EN = PWM → activé via ledcWrite()
    // Ici on s’assure juste que nSLEEP est actif
    digitalWrite(EXSA_DRV_NSLEEP, HIGH);
}

void EXSA_BoosterHw::disableOutput()
{
    // EN = PWM → OFF
    ledcWrite(LEDC_CHANNEL, 0);
    digitalWrite(EXSA_DRV_NSLEEP, LOW);
}

bool EXSA_BoosterHw::isFaultActive()
{
    return digitalRead(EXSA_DRV_FAULT) == LOW; // DRV8874 FAULT = LOW
}

/* ============================================================
 *  PWM DCC — génération du signal voie (EN du DRV8874)
 * ============================================================ */

void EXSA_BoosterHw::setupPwmDcc()
{
    ledcSetup(LEDC_CHANNEL, 20000, 8);     // 20 kHz, 8 bits
    ledcAttachPin(EXSA_DCC_PIN, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, 0);            // PWM OFF au démarrage
}

void EXSA_BoosterHw::applyDcc(const uint8_t *data, uint8_t len)
{
    if (len == 0)
        return;

    // PH = polarité du bit DCC
    digitalWrite(EXSA_DRV_PHASE, data[0] ? HIGH : LOW);

    // EN = PWM pleine puissance
    ledcWrite(LEDC_CHANNEL, 255);
}

/* ============================================================
 *  Cutout RailCom
 * ============================================================ */

void EXSA_BoosterHw::enableCutout()
{
    // EN = PWM OFF → voie flottante
    ledcWrite(LEDC_CHANNEL, 0);
}

void EXSA_BoosterHw::disableCutout()
{
    // EN = PWM ON
    ledcWrite(LEDC_CHANNEL, 255);
}

/* ============================================================
 *  ADC — courant / tension / RailCom
 * ============================================================ */

void EXSA_BoosterHw::setupAdc()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    adc1_config_width(ADC_WIDTH_BIT_12);

    // Courant via IPROPI (GPIO32 → ADC1_CH4)
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);

    // Tension voie (GPIO33 → ADC1_CH5)
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);

    // RailCom HF (GPIO36 → ADC1_CH0)
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
}

/* ============================================================
 *  Mesure courant DRV8874 (IPROPI)
 * ------------------------------------------------------------
 *  Formule :
 *      I = V_IPROPI / (R_IPROPI * A_IPROPI)
 *
 *  Avec :
 *      R_IPROPI = 3,6 kΩ
 *      A_IPROPI = 455 µA/A
 * ============================================================ */

uint16_t EXSA_BoosterHw::readCurrent_mA()
{
    int raw = analogRead(EXSA_ADC_IPROPI);
    float v = (raw / 4095.0f) * 3.3f;

    float currentA = v / (EXSA_IPROPI_R_OHMS * EXSA_IPROPI_GAIN_A_PER_A);
    return (uint16_t)(currentA * 1000.0f);
}

/* ============================================================
 *  Mesure tension voie
 * ============================================================ */

uint16_t EXSA_BoosterHw::readVoltage_mV()
{
    int raw = analogRead(EXSA_ADC_VOLTAGE);

    float v = (raw / 4095.0f) * 3.3f;

    // Diviseur 10k / 47k → facteur 5.7
    float vrail = v * (57.0f / 10.0f);

    return (uint16_t)(vrail * 1000.0f);
}

/* ============================================================
 *  RailCom HF — lecture brute ADC
 * ============================================================ */

int16_t EXSA_BoosterHw::readRailcomAdcRaw()
{
    return adc1_get_raw(ADC1_CHANNEL_0);
}
