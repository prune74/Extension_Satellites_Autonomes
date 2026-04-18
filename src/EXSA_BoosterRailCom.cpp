#include "EXSA_BoosterRailCom.h"
#include "EXSA_BoosterHw.h"
#include <Arduino.h>

/*
 * ============================================================
 *  EXSA_BoosterRailCom — Version Optimisée Discovery 2026
 * ============================================================
 */

volatile bool   EXSA_BoosterRailCom::_active      = false;
volatile int    EXSA_BoosterRailCom::_index       = 0;
int16_t         EXSA_BoosterRailCom::_buffer[BUF_SIZE];
uint16_t        EXSA_BoosterRailCom::_lastAddress = 0;

static hw_timer_t *s_railcomTimer = nullptr;

static void IRAM_ATTR railcomTimerISR()
{
    if (!EXSA_BoosterRailCom::_active)
        return;

    int idx = EXSA_BoosterRailCom::_index;
    if (idx >= EXSA_BoosterRailCom::BUF_SIZE)
        return;

    EXSA_BoosterRailCom::_buffer[idx] = EXSA_BoosterHw::readRailcomAdcRaw();
    EXSA_BoosterRailCom::_index = idx + 1;
}

void EXSA_BoosterRailCom::begin()
{
    _active      = false;
    _index       = 0;
    _lastAddress = 0;

    s_railcomTimer = timerBegin(1, 80, true); // 80 MHz / 80 = 1 MHz
    timerAttachInterrupt(s_railcomTimer, &railcomTimerISR, true);
    timerAlarmWrite(s_railcomTimer, 12, true); // 12 ticks ≈ 12 µs (~83 kHz)
    timerAlarmEnable(s_railcomTimer);
}

void EXSA_BoosterRailCom::onCutoutStart()
{
    _index  = 0;
    _active = true;
}

void EXSA_BoosterRailCom::onCutoutEnd()
{
    _active = false;

    int count = _index;
    if (count < 40)
        return;

    decode();
}

void EXSA_BoosterRailCom::update()
{
    // réservé debug / timeout
}

void EXSA_BoosterRailCom::decode()
{
    int count = _index;
    if (count < 40)
        return;

    int ch1Start = count / 8;
    int ch1Len   = count / 3;

    int ch2Start = ch1Start + ch1Len;
    int ch2Len   = count / 3;

    if (ch1Start + ch1Len > count) ch1Len = count - ch1Start;
    if (ch2Start + ch2Len > count) ch2Len = count - ch2Start;

    uint8_t ch1 = decodeChannel(_buffer, ch1Start, ch1Len);
    uint8_t ch2 = decodeChannel(_buffer, ch2Start, ch2Len);

    if (ch1 != 0)
        _lastAddress = ch1;

    (void)ch2;
}

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

uint16_t EXSA_BoosterRailCom::getLastAddress()
{
    return _lastAddress;
}

void EXSA_BoosterRailCom::clearLastAddress()
{
    _lastAddress = 0;
}
