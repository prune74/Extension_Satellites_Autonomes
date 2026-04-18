#pragma once
#include <stdint.h>

class EXSA_BoosterRailCom
{
public:
    static void begin();
    static void onCutoutStart();
    static void onCutoutEnd();
    static void update();          // appelée toutes les 1 ms
    static uint16_t getLastAddress();
    static void clearLastAddress();

    // Membres accessibles par l’ISR
    static volatile bool   _active;
    static volatile int    _index;
    static const int       BUF_SIZE = 128;
    static int16_t         _buffer[BUF_SIZE];

private:
    static uint16_t        _lastAddress;

    static void decode();
    static uint8_t decodeChannel(const int16_t *buf, int startIndex, int length);
};
