#include "EXSA_BoosterHw.h"
#include "EXSA_Config.h"
#include "EXSA_Pins.h"
#include <Arduino.h>
#include <driver/adc.h>

/*
 * ============================================================
 *  EXSA_BoosterHw — Couche matérielle du Booster Discovery 2026
 * ============================================================
 */

static constexpr int LEDC_CHANNEL = 0;

/* ============================================================
 *  begin() — Initialisation complète du hardware voie
 * ============================================================ */
void EXSA_BoosterHw::begin()
{
    setupDrv8801();   // pont en H
    setupPwmDcc();    // PWM DCC
    setupAdc();       // ADC courant / tension / RailCom
}

/* ============================================================
 *  DRV8801 — Gestion du pont en H
 * ============================================================ */

void EXSA_BoosterHw::setupDrv8801()
{
    pinMode(EXSA_DRV_ENABLE, OUTPUT);
    pinMode(EXSA_DRV_PHASE,  OUTPUT);
    pinMode(EXSA_DRV_FAULT,  INPUT_PULLUP);

    disableOutput();                 // sécurité au démarrage
    digitalWrite(EXSA_DRV_PHASE, LOW);
}

void EXSA_BoosterHw::enableOutput()
{
    digitalWrite(EXSA_DRV_ENABLE, HIGH);
}

void EXSA_BoosterHw::disableOutput()
{
    digitalWrite(EXSA_DRV_ENABLE, LOW);
}

bool EXSA_BoosterHw::isFaultActive()
{
    return digitalRead(EXSA_DRV_FAULT) == LOW; // DRV8801 FAULT = LOW
}

/* ============================================================
 *  PWM DCC — génération du signal voie
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

    digitalWrite(EXSA_DRV_PHASE, data[0] ? HIGH : LOW);
    ledcWrite(LEDC_CHANNEL, 255);          // PWM pleine puissance
}

/* ============================================================
 *  Cutout RailCom
 * ============================================================ */

void EXSA_BoosterHw::enableCutout()
{
    ledcWrite(LEDC_CHANNEL, 0);            // PWM OFF
    digitalWrite(EXSA_DRV_ENABLE, LOW);    // voie flottante
}

void EXSA_BoosterHw::disableCutout()
{
    digitalWrite(EXSA_DRV_ENABLE, HIGH);   // réactive la voie
}

/* ============================================================
 *  ADC — courant / tension / RailCom
 * ============================================================ */

void EXSA_BoosterHw::setupAdc()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    adc1_config_width(ADC_WIDTH_BIT_12);

    // Courant via VPROPI (GPIO32 → ADC1_CH4)
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);

    // Tension voie (GPIO33 → ADC1_CH5)
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);

    // RailCom HF (GPIO36 → ADC1_CH0)
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
}

uint16_t EXSA_BoosterHw::readCurrent_mA()
{
    int raw = analogRead(EXSA_ADC_VPROPI);

    float v = (raw / 4095.0f) * 3.3f;

    // VPROPI = 5 × Vshunt  →  I = VPROPI / (5 × Rshunt)
    float current = (v / (5.0f * EXSA_SHUNT_OHMS)) * 1000.0f;

    return (uint16_t)current;
}

uint16_t EXSA_BoosterHw::readVoltage_mV()
{
    int raw = analogRead(EXSA_ADC_VOLTAGE);

    float v = (raw / 4095.0f) * 3.3f;
    float vrail = v * (57.0f / 10.0f); // diviseur 10k / 47k

    return (uint16_t)(vrail * 1000.0f);
}

int16_t EXSA_BoosterHw::readRailcomAdcRaw()
{
    return adc1_get_raw(ADC1_CHANNEL_0);
}
