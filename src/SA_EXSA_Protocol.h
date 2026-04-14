#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  ENUM DES ASPECTS SNCF — Option A (1 octet)
 * ------------------------------------------------------------
 *  Ce type est utilisé par :
 *    - SupervisionCanton (SA)
 *    - DeductionAspect (SA)
 *    - AspectSignal (SA)
 *    - EXSA_Main (EXSA)
 *    - EXSA_Signaux (EXSA)
 *
 *  ⚠️ C’est la source de vérité unique pour les aspects SNCF.
 * ============================================================
 */

enum ExsaAspect : uint8_t {
    ASPECT_CARRE = 0,
    ASPECT_SEMAPHORE,
    ASPECT_AVERTISSEMENT,
    ASPECT_RALENTISSEMENT_30,
    ASPECT_RALENTISSEMENT_60,
    ASPECT_RAPPEL_30,
    ASPECT_RAPPEL_60,
    ASPECT_VOIE_LIBRE,
    ASPECT_MANOEUVRE,
    ASPECT_MASQUE,
    ASPECT_DEFAUT
};

/*
 * ============================================================
 *  PROTOCOLE OFFICIEL SA ↔ EXSA — Version ENUM (Option A)
 * ------------------------------------------------------------
 *  Format général des trames :
 *
 *      [SYNC = 0xAA][OPCODE][DATA...]
 *
 *  Les aspects SNCF sont envoyés en 1 octet (ExsaAspect).
 * ============================================================
 */

#define PROTO_SYNC_BYTE 0xAA

/* ============================================================
 *  🟦 SA → EXSA : Commandes envoyées par le Satellite (SA)
 * ============================================================
 */

/* --- Supervision / Vie du module --- */
#define PROTO_PING                      0x32   // SA → EXSA : ping
#define PROTO_PONG                      0x33   // EXSA → SA : pong (réponse)

/* --- Topologie et configuration --- */
#define PROTO_E4_TOPOLOGIE_CAN          0xE4
#define PROTO_E5_CONFIG_SIGNAUX         0xE5

/* --- Aspects SNCF (1 octet = enum ExsaAspect) --- */
#define PROTO_E6_ASPECT_HORAIRE         0xE6
#define PROTO_E7_ASPECT_ANTIHORAIRE     0xE7

/* --- Feux directionnels (barrettes cumulatives) --- */
#define PROTO_E8_DIRECTION_HORAIRE      0xE8
#define PROTO_E9_DIRECTION_ANTIHORAIRE  0xE9

/* --- Occupation des voisins SP1 / SM1 --- */
#define PROTO_EA_OCCUPATION_VOISINS     0xEA

/* --- Commandes servos EXSA --- */
#define PROTO_F0_SERVO_MOVE             0xF0
#define PROTO_F1_SERVO_CONFIG           0xF1
#define PROTO_F2_SERVO_TEST             0xF2

/* ============================================================
 *  🟩 EXSA → SA : Informations envoyées par l’EXSA
 * ============================================================
 */

/* --- Capteurs ponctuels (ILS / IR) --- */
#define PROTO_03_PONCTUEL               0x03

/* --- Occupation canton (présence) --- */
#define PROTO_04_OCCUPATION             0x04

/* --- Delta axe (comptage essieux) --- */
#define PROTO_05_DELTA_AXE              0x05

/* --- Position réelle des aiguilles (micro-switchs) --- */
#define PROTO_06_POSITION_AIGUILLE      0x06

/* ============================================================
 *  🟪 Codes associés (EXSA → SA)
 * ============================================================
 */

/* --- PONCTUEL (côté H / AH) --- */
#define PROTO_PONCT_H_ACTIVE            0x10
#define PROTO_PONCT_H_INACTIVE          0x11
#define PROTO_PONCT_AH_ACTIVE           0x12
#define PROTO_PONCT_AH_INACTIVE         0x13

/* --- OCCUPATION canton --- */
#define PROTO_OCC_ACTIVE                0x30
#define PROTO_OCC_LIBRE                 0x31

/* --- DELTA AXE (+1 / -1) --- */
#define PROTO_DELTA_PLUS_UN             0x01
#define PROTO_DELTA_MOINS_UN            0xFF

/* --- Codes position réelle --- */
#define PROTO_POS_DROIT        0x00
#define PROTO_POS_DEVIE        0x01
#define PROTO_POS_INDET        0x02
#define PROTO_POS_INCOHERENT   0x03

/* --- Codes état sécurité --- */
#define PROTO_ETAT_OK          0x00
#define PROTO_ETAT_BLOQUE      0x01
#define PROTO_ETAT_ERREUR      0x02

/* ============================================================
 *  Fin du fichier SA_EXSA_Protocol.h — Version ENUM + PING/PONG
 * ============================================================
 */
