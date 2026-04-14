/* ============================================================
   EXSA_UartRx.cpp — Réception UART du protocole SA → EXSA
   Version ENUM (Option A)
   ------------------------------------------------------------
   Rôle :
     - Lire les trames envoyées par le SA
     - Parser opcode + données
     - Appeler les callbacks EXSA_Main correspondants

   Format des trames :
       [SYNC=0xAA] [OPCODE] [DATA...]

   Caractéristiques :
     - parser minimaliste, robuste, sans malloc
     - reset automatique en cas d’erreur
     - Aspects SNCF = 1 octet (enum ExsaAspect)
============================================================ */

#include "EXSA_UartRx.h"
#include "EXSA_Main.h"
#include "EXSA_Config.h"
#include "EXSA_UartTx.h"   // ← nécessaire pour envoyer PONG

static HardwareSerial* uart = nullptr;

/* ============================================================
   Buffer interne du parser
============================================================ */
static uint8_t rxState  = 0;     // 0 = attente SYNC, 1 = opcode, 2 = data
static uint8_t rxOpcode = 0;
static uint8_t rxData[8];
static uint8_t rxIndex  = 0;

/* ============================================================
   Initialisation UART
============================================================ */
void EXSA_UartRx::begin(HardwareSerial& serial, uint32_t baudrate) noexcept
{
    uart = &serial;
    uart->begin(baudrate, SERIAL_8N1, EXSA_UART_RX_PIN, EXSA_UART_TX_PIN);
}

/* ============================================================
   Helpers internes
============================================================ */
static inline void resetParser() noexcept
{
    rxState  = 0;
    rxOpcode = 0;
    rxIndex  = 0;
}

/*
 * handleFrame()
 * ------------------------------------------------------------
 * Appelle la fonction EXSA_Main correspondante à l’opcode.
 * Les données sont déjà validées et de longueur correcte.
 */
static inline void handleFrame(uint8_t opcode, uint8_t* data, uint8_t len) noexcept
{
    switch (opcode)
    {
        /* -----------------------------
           PING → répondre PONG
        ------------------------------*/
        case PROTO_PING:
            // data[0] = index EXSA (0 ou 1)
            EXSA_UartTx::envoyerPong(data[0]);
            break;

        case PROTO_E4_TOPOLOGIE_CAN:
            EXSA_Main::onTopologie(data, len);
            break;

        case PROTO_E5_CONFIG_SIGNAUX:
            EXSA_Main::onConfigSignaux(data, len);
            break;

        case PROTO_E6_ASPECT_HORAIRE:
            EXSA_Main::onAspectHoraire(data[0]);
            break;

        case PROTO_E7_ASPECT_ANTIHORAIRE:
            EXSA_Main::onAspectAntiHoraire(data[0]);
            break;

        case PROTO_E8_DIRECTION_HORAIRE:
            EXSA_Main::onDirectionHoraire(data[0]);
            break;

        case PROTO_E9_DIRECTION_ANTIHORAIRE:
            EXSA_Main::onDirectionAntiHoraire(data[0]);
            break;

        case PROTO_EA_OCCUPATION_VOISINS:
            EXSA_Main::onOccupationVoisins(data[0]);
            break;

        case PROTO_F0_SERVO_MOVE:
            EXSA_Main::onServoMove(data[0], data[1]);
            break;

        case PROTO_F1_SERVO_CONFIG:
            EXSA_Main::onServoConfig(
                data[0],
                uint16_t(data[1] | (data[2] << 8)),
                uint16_t(data[3] | (data[4] << 8)),
                uint16_t(data[5] | (data[6] << 8))
            );
            break;

        case PROTO_F2_SERVO_TEST:
            EXSA_Main::onServoTest(data[0]);
            break;

        default:
            break;
    }
}

/* ============================================================
   process() — Lecture et parsing UART
============================================================ */
void EXSA_UartRx::process() noexcept
{
    if (!uart)
        return;

    while (uart->available())
    {
        const uint8_t b = uart->read();

        switch (rxState)
        {
            case 0: // Attente SYNC
                if (b == PROTO_SYNC_BYTE)
                    rxState = 1;
                break;

            case 1: // Lecture opcode
                rxOpcode = b;
                rxIndex  = 0;
                rxState  = 2;
                break;

            case 2: // Lecture DATA
            {
                rxData[rxIndex++] = b;

                uint8_t expectedLen = 0;

                switch (rxOpcode)
                {
                    /* -----------------------------
                       Longueur PING = 1 octet
                    ------------------------------*/
                    case PROTO_PING:                     expectedLen = 1;  break;

                    case PROTO_E4_TOPOLOGIE_CAN:         expectedLen = 10; break;
                    case PROTO_E5_CONFIG_SIGNAUX:        expectedLen = 4;  break;

                    case PROTO_E6_ASPECT_HORAIRE:        expectedLen = 1;  break;
                    case PROTO_E7_ASPECT_ANTIHORAIRE:    expectedLen = 1;  break;

                    case PROTO_E8_DIRECTION_HORAIRE:     expectedLen = 1;  break;
                    case PROTO_E9_DIRECTION_ANTIHORAIRE: expectedLen = 1;  break;
                    case PROTO_EA_OCCUPATION_VOISINS:    expectedLen = 1;  break;

                    case PROTO_F0_SERVO_MOVE:            expectedLen = 2;  break;
                    case PROTO_F1_SERVO_CONFIG:          expectedLen = 7;  break;
                    case PROTO_F2_SERVO_TEST:            expectedLen = 1;  break;

                    default:
                        expectedLen = 0;
                        break;
                }

                if (rxIndex >= expectedLen)
                {
                    handleFrame(rxOpcode, rxData, expectedLen);
                    resetParser();
                }
                break;
            }

            default:
                resetParser();
                break;
        }
    }
}

/* ============================================================
   Fin de EXSA_UartRx.cpp — Version ENUM + PING/PONG
============================================================ */
