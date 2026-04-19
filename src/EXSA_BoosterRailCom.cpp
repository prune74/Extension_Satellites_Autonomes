#include "EXSA_BoosterRailCom.h"
#include "EXSA_BoosterHw.h"
#include <Arduino.h>

/*
 * ============================================================
 *  EXSA_BoosterRailCom — Décodage RailCom Discovery 2026
 * ============================================================
 */

// ------------------------------------------------------------
// Variables internes (partagées avec l’ISR)
// ------------------------------------------------------------
volatile bool   EXSA_BoosterRailCom::_active      = false;
volatile int    EXSA_BoosterRailCom::_index       = 0;
int16_t         EXSA_BoosterRailCom::_buffer[BUF_SIZE];

uint16_t        EXSA_BoosterRailCom::_lastAddress = 0;

// ------------------------------------------------------------
// Filtrage anti-parasites (4 valeurs identiques)
// ------------------------------------------------------------
static const int FILTER_SIZE = 4;
static uint16_t filterBuf[FILTER_SIZE] = {0};
static int filterIndex = 0;

// ------------------------------------------------------------
// Timer matériel pour ISR RailCom
// ------------------------------------------------------------
static hw_timer_t *s_railcomTimer = nullptr;

/* ============================================================
 *  ISR — échantillonnage ADC haute fréquence (~83 kHz)
 * ============================================================ */

static void IRAM_ATTR railcomTimerISR()
{
    if (!EXSA_BoosterRailCom::_active)
        return;

    int idx = EXSA_BoosterRailCom::_index;
    if (idx >= EXSA_BoosterRailCom::BUF_SIZE)
        return;

    // Lecture RailCom HF → ADC1_CH0 (GPIO36)
    EXSA_BoosterRailCom::_buffer[idx] = EXSA_BoosterHw::readRailcomAdcRaw();
    EXSA_BoosterRailCom::_index = idx + 1;
}

/* ============================================================
 *  begin() — Initialisation RailCom
 * ============================================================ */

void EXSA_BoosterRailCom::begin()
{
    _active      = false;
    _index       = 0;
    _lastAddress = 0;

    s_railcomTimer = timerBegin(1, 80, true); // 80 MHz / 80 = 1 MHz
    timerAttachInterrupt(s_railcomTimer, &railcomTimerISR, true);
    timerAlarmWrite(s_railcomTimer, 12, true); // 1 MHz / 12 ≈ 83 kHz
    timerAlarmEnable(s_railcomTimer);
}

/* ============================================================
 *  Début / fin de cutout
 * ============================================================ */

void EXSA_BoosterRailCom::onCutoutStart()
{
    _index  = 0;
    _active = true;
}

void EXSA_BoosterRailCom::onCutoutEnd()
{
    _active = false;

    if (_index < 40)
        return;

    decode();
}

void EXSA_BoosterRailCom::update()
{
    // réservé debug / extensions
}

/* ============================================================
 *  decode() — Décodage Locoduino CH1 / CH2
 * ============================================================ */

void EXSA_BoosterRailCom::decode()
{
    int count = _index;
    if (count < 40)
        return;

    // Découpage CH1 / CH2
    int ch1Start = count / 8;
    int ch1Len   = count / 3;

    int ch2Start = ch1Start + ch1Len;
    int ch2Len   = count / 3;

    if (ch1Start + ch1Len > count) ch1Len = count - ch1Start;
    if (ch2Start + ch2Len > count) ch2Len = count - ch2Start;

    uint8_t ch1 = decodeChannel(_buffer, ch1Start, ch1Len);
    uint8_t ch2 = decodeChannel(_buffer, ch2Start, ch2Len);

    if (ch1 == 0)
        return;

    uint8_t header = ch1;
    uint8_t data   = ch2;

    const uint8_t CH1_ADR_LOW  = 1 << 2;
    const uint8_t CH1_ADR_HIGH = 1 << 3;

    static int16_t dccAddrLow  = -1;
    static int16_t dccAddrHigh = -1;

    if (header & CH1_ADR_LOW)
        dccAddrLow = data | (header << 6);

    if (header & CH1_ADR_HIGH)
        dccAddrHigh = data | (header << 6);

    if (dccAddrLow < 0 || dccAddrHigh < 0)
        return;

    int16_t addr;

    if (dccAddrHigh < 128)
        addr = dccAddrLow;
    else
        addr = ((dccAddrHigh - 128) << 8) + dccAddrLow;

    // Filtrage anti-parasites
    filterBuf[filterIndex] = addr;
    filterIndex = (filterIndex + 1) % FILTER_SIZE;

    for (int i = 0; i < FILTER_SIZE; i++)
    {
        if (filterBuf[i] != addr)
            return;
    }

    _lastAddress = addr;

    dccAddrLow  = -1;
    dccAddrHigh = -1;
}

/* ============================================================
 *  decodeChannel() — Décodage d’un canal RailCom (8 bits)
 * ============================================================ */

uint8_t EXSA_BoosterRailCom::decodeChannel(const int16_t *buf,
                                           int startIndex,
                                           int length)
{
    if (length < 20)
        return 0;

    int16_t minV = 32767;
    int16_t maxV = -32768;

    for (int i = 0; i < length; i++)
    {
        int16_t v = buf[startIndex + i];
        if (v < minV) minV = v;
        if (v > maxV) maxV = v;
    }

    int16_t threshold = (minV + maxV) / 2;

    int startBitIndex = -1;
    int searchLimit = min(20, length - 1);

    for (int i = 1; i < searchLimit; i++)
    {
        int16_t prev = buf[startIndex + i - 1];
        int16_t curr = buf[startIndex + i];

        if (prev > threshold && curr < threshold)
        {
            startBitIndex = i;
            break;
        }
    }

    if (startBitIndex < 0)
        return 0;

    const int samplesPerBit = 2;
    uint8_t value = 0;

    for (int bit = 0; bit < 8; bit++)
    {
        int sampleIndex =
            startIndex + startBitIndex + (bit + 1) * samplesPerBit + 1;

        if (sampleIndex >= startIndex + length)
            break;

        if (buf[sampleIndex] < threshold)
            value |= (1 << bit);
    }

    return value;
}

/* ============================================================
 *  API publique
 * ============================================================ */

uint16_t EXSA_BoosterRailCom::getLastAddress()
{
    return _lastAddress;
}

void EXSA_BoosterRailCom::clearLastAddress()
{
    _lastAddress = 0;
}
