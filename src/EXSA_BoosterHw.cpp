#include "EXSA_BoosterHw.h"
#include "EXSA_Config.h"
#include "EXSA_Pins.h"
#include <Arduino.h>
#include <driver/adc.h>

/*
 * ============================================================
 *  EXSA_BoosterHw — Optimisé Discovery 2026
 * ============================================================
 */

static constexpr int LEDC_CHANNEL = 0;

void EXSA_BoosterHw::begin()
{
    setupDrv8801();
    setupPwmDcc();
    setupAdc();
}

void EXSA_BoosterHw::setupDrv8801()
{
    pinMode(EXSA_DRV_ENABLE, OUTPUT);
    pinMode(EXSA_DRV_PHASE,  OUTPUT);
    pinMode(EXSA_DRV_FAULT,  INPUT_PULLUP);

    disableOutput();
    digitalWrite(EXSA_DRV_PHASE, LOW);
}

void EXSA_BoosterHw::setupPwmDcc()
{
    ledcSetup(LEDC_CHANNEL, 20000, 8);
    ledcAttachPin(EXSA_DCC_PIN, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, 0);
}

void EXSA_BoosterHw::setupAdc()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
}

void EXSA_BoosterHw::applyDcc(const uint8_t *data, uint8_t len)
{
    if (len == 0)
        return;

    if (data[0])
        digitalWrite(EXSA_DRV_PHASE, HIGH);
    else
        digitalWrite(EXSA_DRV_PHASE, LOW);

    ledcWrite(LEDC_CHANNEL, 255);
}

void EXSA_BoosterHw::enableCutout()
{
    ledcWrite(LEDC_CHANNEL, 0);
    digitalWrite(EXSA_DRV_ENABLE, LOW);
}

void EXSA_BoosterHw::disableCutout()
{
    digitalWrite(EXSA_DRV_ENABLE, HIGH);
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
    return digitalRead(EXSA_DRV_FAULT) == LOW;
}

uint16_t EXSA_BoosterHw::readCurrent_mA()
{
    int raw = analogRead(EXSA_ADC_CURRENT);

    float voltage = (raw / 4095.0f) * 3.3f;
    float current = (voltage / 0.14f) * 1000.0f;

    return (uint16_t)current;
}

uint16_t EXSA_BoosterHw::readVoltage_mV()
{
    int raw = analogRead(EXSA_ADC_VOLTAGE);

    float v = (raw / 4095.0f) * 3.3f;
    float vrail = v * (57.0f / 10.0f);

    return (uint16_t)(vrail * 1000.0f);
}

int16_t EXSA_BoosterHw::readRailcomAdcRaw()
{
    // Version rapide (ESP32 ADC1 direct, GPIO32 = ADC1_CH4)
    return adc1_get_raw(ADC1_CHANNEL_4);
}
