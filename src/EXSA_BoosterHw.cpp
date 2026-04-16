#include "EXSA_BoosterHw.h"
#include "EXSA_Config.h"
#include <Arduino.h>

// ------------------------------------------------------------
// Initialisation hardware
// ------------------------------------------------------------
void EXSA_BoosterHw::begin()
{
    setupDrv8801();
    setupPwmDcc();
    setupAdc();
}

// ------------------------------------------------------------
// Configuration DRV8801
// ------------------------------------------------------------
void EXSA_BoosterHw::setupDrv8801()
{
    pinMode(EXSA_DRV_ENABLE, OUTPUT);
    pinMode(EXSA_DRV_PHASE,  OUTPUT);
    pinMode(EXSA_DRV_FAULT,  INPUT_PULLUP);

    disableOutput();
}

// ------------------------------------------------------------
// Configuration PWM DCC
// ------------------------------------------------------------
void EXSA_BoosterHw::setupPwmDcc()
{
    // Exemple : canal LEDC 0, fréquence 20 kHz, résolution 8 bits
    ledcSetup(0, 20000, 8);
    ledcAttachPin(EXSA_DCC_PIN, 0);

    // PWM à 0 au démarrage
    ledcWrite(0, 0);
}

// ------------------------------------------------------------
// Configuration ADC (courant + tension voie)
// ------------------------------------------------------------
void EXSA_BoosterHw::setupAdc()
{
    analogReadResolution(12); // 0–4095
    analogSetAttenuation(ADC_11db);
}

// ------------------------------------------------------------
// Application d’une trame DCC (simplifiée)
// ------------------------------------------------------------
void EXSA_BoosterHw::applyDcc(const uint8_t *data, uint8_t len)
{
    // ⚠️ SQUELETTE : ici tu mettras ta génération DCC réelle
    // Pour l’instant : simple PWM ON/OFF

    if (len == 0)
        return;

    // Exemple : si premier octet pair → ON, impair → OFF
    if (data[0] & 1)
        ledcWrite(0, 255); // ON
    else
        ledcWrite(0, 0);   // OFF
}

// ------------------------------------------------------------
// Cutout RailCom
// ------------------------------------------------------------
void EXSA_BoosterHw::enableCutout()
{
    // Exemple : PWM OFF pendant cutout
    ledcWrite(0, 0);
}

void EXSA_BoosterHw::disableCutout()
{
    // Exemple : PWM ON après cutout
    ledcWrite(0, 255);
}

// ------------------------------------------------------------
// DRV8801 : activation / désactivation voie
// ------------------------------------------------------------
void EXSA_BoosterHw::enableOutput()
{
    digitalWrite(EXSA_DRV_ENABLE, HIGH);
}

void EXSA_BoosterHw::disableOutput()
{
    digitalWrite(EXSA_DRV_ENABLE, LOW);
}

// ------------------------------------------------------------
// Lecture courant voie (mA)
// ------------------------------------------------------------
uint16_t EXSA_BoosterHw::readCurrent_mA()
{
    int raw = analogRead(EXSA_ADC_CURRENT);
    // ⚠️ SQUELETTE : conversion à ajuster selon ton shunt
    return raw; 
}

// ------------------------------------------------------------
// Lecture tension voie (mV)
// ------------------------------------------------------------
uint16_t EXSA_BoosterHw::readVoltage_mV()
{
    int raw = analogRead(EXSA_ADC_VOLTAGE);
    // ⚠️ SQUELETTE : conversion à ajuster selon ton pont diviseur
    return raw;
}
