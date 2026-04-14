#pragma once
#include <Arduino.h>
#include "EXSA_Config.h"          // Pins UART + configuration globale
#include "SA_EXSA_Protocol.h"     // Protocole officiel SA ↔ EXSA

/*
 * ============================================================
 *  EXSA_UartRx.h — Réception UART SA → EXSA
 *  Version ENUM (Option A)
 * ------------------------------------------------------------
 *  Rôle :
 *    - Lire les trames envoyées par le SA
 *    - Vérifier le byte de synchro (0xAA)
 *    - Lire l’opcode
 *    - Lire la quantité de données attendue selon l’opcode
 *    - Appeler la fonction EXSA_Main correspondante
 *
 *  Format d’une trame SA → EXSA :
 *      [0xAA] [OPCODE] [DATA...]
 *
 *  Caractéristiques :
 *    - parser simple et robuste
 *    - pas de CRC
 *    - pas d’allocation dynamique
 *    - reset automatique en cas d’erreur
 *
 *  ⚠️ Aspects SNCF :
 *     → 1 octet = enum ExsaAspect
 *
 *  ⚠️ PING / PONG :
 *     - PROTO_PING (0x30) : envoyé par SA
 *     - EXSA répond PROTO_PONG (0x31) via EXSA_UartTx
 *     - EXSA reste 100 % passif : il ne parle jamais en premier
 *
 *  EXSA_Main::loop() doit appeler process() régulièrement.
 * ============================================================
 */

namespace EXSA_UartRx
{
    /*
     * begin()
     * ---------------------------------------------------------
     * Initialise l’UART matériel utilisé pour la communication
     * SA → EXSA.
     *
     * Exemple :
     *     EXSA_UartRx::begin(Serial1, 115200);
     */
    void begin(HardwareSerial& serial, uint32_t baudrate) noexcept;

    /*
     * process()
     * ---------------------------------------------------------
     * À appeler régulièrement dans EXSA_Main::loop().
     *
     * Rôle :
     *   - lire les octets disponibles
     *   - parser les trames complètes
     *   - déclencher les callbacks EXSA_Main
     *   - répondre PONG si SA envoie un PING
     */
    void process() noexcept;
}

/* ============================================================
   Fin de EXSA_UartRx.h — Version ENUM + PING/PONG
   ============================================================ */
