#pragma once
#include <stdint.h>
#include <ACAN_ESP32.h>


/*
 * ============================================================
 *  EXSA_CanBooster.h
 * ------------------------------------------------------------
 *  Module CAN Booster — Discovery 2026
 *
 *  Rôle :
 *    - Réception des trames CAN envoyées par LaBox :
 *         • 0x100 : bit DCC + phase
 *         • 0x101 : cutout local + cutout global
 *         • 0x102 : télémétrie d’un autre booster
 *         • 0x103 : adresse RailCom d’un autre booster
 *
 *    - Mise à disposition des données pour EXSA_Booster :
 *         • dccBuffer / dccLen
 *         • cutoutActive
 *         • railcomAddress
 *         • voieCourant_mA / voieTension_mV
 *         • boosterState (état local ou reçu)
 *
 *    - Gestion des sécurités globales multi-boosters :
 *         • globalFault     (un autre booster est en court-circuit)
 *         • globalOverheat  (un autre booster surchauffe)
 *         • globalOff       (un autre booster a coupé sa voie)
 *         • phaseMismatch   (inversion de phase détectée)
 *         • globalCutout    (cutout global actif)
 *
 *  Ce module ne génère rien : il ne fait que recevoir.
 *  Toute la logique de sécurité est appliquée dans EXSA_Booster.
 * ============================================================
 */

class EXSA_CanBooster
{
public:
    /*
     * --------------------------------------------------------
     * begin()
     * --------------------------------------------------------
     * Initialise le contrôleur CAN (ACAN_ESP32).
     * Vitesse : 500 kbps (réseau Discovery 2026).
     * Broches RX/TX définies dans le .cpp.
     */
    static void begin();

    /*
     * --------------------------------------------------------
     * process()
     * --------------------------------------------------------
     * Boucle de réception CAN.
     * À appeler toutes les 1 ms depuis BoosterCAN_Task.
     * Décode les trames et appelle les handlers internes.
     */
    static void process();

    /*
     * --------------------------------------------------------
     * AJOUT : envoi RailCom (ID 0x103)
     * --------------------------------------------------------
     */
    static void sendRailcomAddress(uint16_t addr);

    /*
     * --------------------------------------------------------
     * Données partagées avec EXSA_Booster
     * --------------------------------------------------------
     * dccBuffer / dccLen :
     *    bit DCC courant (LaBox → Booster)
     *
     * cutoutActive :
     *    cutout local (fenêtre RailCom)
     *
     * railcomAddress :
     *    dernière adresse RailCom reçue d’un autre booster
     *
     * voieCourant_mA / voieTension_mV :
     *    télémétrie reçue d’un autre booster (CAN 0x102)
     *
     * boosterState :
     *    état du booster émetteur (0=OK, 1=CC, 2=TH, 3=OFF)
     *    utilisé pour la sécurité globale.
     *
     * NOTE :
     *    boosterState local est mis à jour dans EXSA_Booster,
     *    pas ici (les trames reçues ne modifient pas l’état local).
     */
    static volatile uint8_t  dccBuffer[8];
    static volatile uint8_t  dccLen;

    static volatile bool     cutoutActive;
    static volatile uint16_t railcomAddress;

    static volatile uint16_t voieCourant_mA;
    static volatile uint16_t voieTension_mV;
    static volatile uint8_t  boosterState;

    /*
     * --------------------------------------------------------
     * Sécurités globales Discovery 2026
     * --------------------------------------------------------
     * Ces flags sont mis à jour dans :
     *    - onTelemetryFrame()
     *    - onCutoutFrame()
     *    - onDccFrame()
     *
     * Ils sont lus dans EXSA_Booster::checkSafety().
     */
    static volatile bool globalFault;      // un autre booster est en court-circuit
    static volatile bool globalOverheat;   // un autre booster surchauffe
    static volatile bool globalOff;        // un autre booster a coupé sa voie
    static volatile bool phaseMismatch;    // inversion de phase détectée
    static volatile bool globalCutout;     // cutout global actif

private:
    /*
     * --------------------------------------------------------
     * handleFrame()
     * --------------------------------------------------------
     * Dispatch des trames CAN selon leur ID.
     */
    static void handleFrame(const CANMessage &msg);

    /*
     * --------------------------------------------------------
     * Handlers individuels (appelés par handleFrame)
     * --------------------------------------------------------
     */
    static void onDccFrame(const CANMessage &msg);       // 0x100
    static void onCutoutFrame(const CANMessage &msg);    // 0x101
    static void onTelemetryFrame(const CANMessage &msg); // 0x102
    static void onRailcomFrame(const CANMessage &msg);   // 0x103
};
