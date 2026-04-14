#include "EXSA_Switches.h"
#include "EXSA_UartTx.h"
#include "EXSA_Servo.h"

/*
 * ============================================================
 *  Variables statiques
 * ------------------------------------------------------------
 *  Ces tableaux mémorisent :
 *    - les derniers états envoyés au SA
 *    - les timers d’envoi périodique
 *    - les timers de détection de blocage
 *
 *  Ils sont définis ici (et non dans le header) car ils ne
 *  doivent exister qu’une seule fois dans tout le programme.
 * ============================================================
 */

Adafruit_MCP23X17 EXSA_Switches::mcp;

/*
 * Définition des tableaux constexpr déclarés dans le .h
 * ------------------------------------------------------------
 * ⚠️ Obligatoire sur ESP32 / GCC 8.4.0 :
 *    Un tableau static constexpr DOIT avoir une définition
 *    dans un .cpp, sinon → undefined reference.
 */
constexpr uint8_t EXSA_Switches::swDroit[AIG_COUNT] = {
    EXSA_SW0_DROIT,
    EXSA_SW1_DROIT,
    EXSA_SW2_DROIT
};

constexpr uint8_t EXSA_Switches::swDevie[AIG_COUNT] = {
    EXSA_SW0_DEVIE,
    EXSA_SW1_DEVIE,
    EXSA_SW2_DEVIE
};

// Derniers états envoyés au SA (position + état sécurité)
uint8_t  EXSA_Switches::lastPos[AIG_COUNT]      = {255,255,255};
uint8_t  EXSA_Switches::lastEtat[AIG_COUNT]     = {255,255,255};
uint32_t EXSA_Switches::lastSendMs[AIG_COUNT]   = {0,0,0};

// Suivi du mouvement pour détection blocage
uint32_t EXSA_Switches::moveStartMs[AIG_COUNT]  = {0,0,0};
uint32_t EXSA_Switches::moveMaxMs[AIG_COUNT]    = {0,0,0};
bool     EXSA_Switches::movementActive[AIG_COUNT] = {false,false,false};

/*
 * ============================================================
 *  Helper : calcul du temps max de mouvement
 * ------------------------------------------------------------
 *  On estime la durée max en fonction de l’amplitude PWM :
 *
 *      amplitude = |current_pwm - target_pwm|
 *
 *  Puis :
 *      temps ≈ amplitude / 2   (≈ 0.5 ms par pas PWM)
 *
 *  Borné entre 200 et 1500 ms, puis +200 ms de marge.
 *
 *  Ce n’est pas une science exacte, mais suffisant pour
 *  détecter un servo bloqué ou mécanique dure.
 * ============================================================
 */
static uint32_t calculerTempsMaxMouvement(uint8_t idx)
{
    uint16_t cur = EXSA_Servo::getCurrentPwm(idx);
    uint16_t tgt = EXSA_Servo::getTargetPwm(idx);

    if (cur == 0 || tgt == 0)
        return 800; // valeur de secours

    uint16_t amplitude = (cur > tgt) ? (cur - tgt) : (tgt - cur);

    uint32_t temps_theorique = amplitude / 2;

    if (temps_theorique < 200)  temps_theorique = 200;
    if (temps_theorique > 1500) temps_theorique = 1500;

    return temps_theorique + 200; // marge
}

/*
 * ============================================================
 *  begin()
 * ------------------------------------------------------------
 *  Initialise le MCP23017 :
 *    - communication I²C
 *    - configuration des broches en entrée
 *    - activation des pull-up internes
 *
 *  Les micro-switchs sont câblés en "actif bas".
 * ============================================================
 */
void EXSA_Switches::begin()
{
    mcp.begin_I2C(EXSA_MCP23017_ADDR);

    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        mcp.pinMode(swDroit[i], INPUT_PULLUP);
        mcp.pinMode(swDevie[i], INPUT_PULLUP);
    }
}

/*
 * ============================================================
 *  notifierMouvementDemarre()
 * ------------------------------------------------------------
 *  Appelé par EXSA_Main lorsqu’un F0 est reçu.
 *
 *  Active la surveillance :
 *    - movementActive = true
 *    - moveStartMs    = maintenant
 *    - moveMaxMs      = calcul dynamique via PWM
 * ============================================================
 */
void EXSA_Switches::notifierMouvementDemarre(uint8_t idx)
{
    if (idx >= AIG_COUNT)
        return;

    movementActive[idx] = true;
    moveStartMs[idx]    = millis();
    moveMaxMs[idx]      = calculerTempsMaxMouvement(idx);
}

/*
 * ============================================================
 *  lirePosition()
 * ------------------------------------------------------------
 *  Lit les deux micro-switchs :
 *    - droit = !digitalRead(swDroit)
 *    - devie = !digitalRead(swDevie)
 *
 *  Déduction :
 *    - 1 seul actif → DROIT / DEVIE
 *    - aucun actif → INDET
 *    - les deux actifs → INCOHERENT
 * ============================================================
 */
uint8_t EXSA_Switches::lirePosition(uint8_t idx)
{
    bool droit = !mcp.digitalRead(swDroit[idx]);
    bool devie = !mcp.digitalRead(swDevie[idx]);

    if (droit && !devie) return PROTO_POS_DROIT;
    if (!droit && devie) return PROTO_POS_DEVIE;
    if (!droit && !devie) return PROTO_POS_INDET;
    return PROTO_POS_INCOHERENT;
}

/*
 * ============================================================
 *  lireEtat()
 * ------------------------------------------------------------
 *  Déduit l’état sécurité :
 *
 *    - INCOHERENT → ERREUR
 *    - INDET      → ERREUR
 *
 *    - BLOQUE si :
 *         movementActive == true
 *         ET (temps > moveMaxMs)
 *
 *    - Sinon → OK
 * ============================================================
 */
uint8_t EXSA_Switches::lireEtat(uint8_t idx, uint8_t pos)
{
    if (pos == PROTO_POS_INCOHERENT)
        return PROTO_ETAT_ERREUR;

    if (pos == PROTO_POS_INDET)
        return PROTO_ETAT_ERREUR;

    if (movementActive[idx])
    {
        uint32_t now = millis();
        if (now - moveStartMs[idx] > moveMaxMs[idx])
            return PROTO_ETAT_BLOQUE;
    }

    return PROTO_ETAT_OK;
}

/*
 * ============================================================
 *  envoyerTrame()
 * ------------------------------------------------------------
 *  Envoie la trame 0x06 :
 *
 *      [AA][06][index][position][etat]
 *
 *  Met aussi à jour lastSendMs pour l’envoi périodique.
 * ============================================================
 */
void EXSA_Switches::envoyerTrame(uint8_t idx, uint8_t pos, uint8_t etat)
{
    EXSA_UartTx::envoyerTramePositionAiguille(idx, pos, etat);
    lastSendMs[idx] = millis();
}

/*
 * ============================================================
 *  update()
 * ------------------------------------------------------------
 *  Appelé toutes les 10–20 ms dans EXSA_Main.
 *
 *  Rôle :
 *    - Lire la position réelle
 *    - Déduire l’état sécurité
 *    - Détecter un changement
 *    - Envoyer immédiatement si changement
 *    - Envoyer périodiquement toutes les 200 ms
 *
 *  Fin de mouvement :
 *    - si pos valide + OK → movementActive = false
 * ============================================================
 */
void EXSA_Switches::update()
{
    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        uint8_t pos  = lirePosition(i);
        uint8_t etat = lireEtat(i, pos);

        // Fin de mouvement : position atteinte + OK
        if ((pos == PROTO_POS_DROIT || pos == PROTO_POS_DEVIE) &&
            etat == PROTO_ETAT_OK)
        {
            movementActive[i] = false;
        }

        bool changed  = (pos != lastPos[i]) || (etat != lastEtat[i]);
        bool periodic = (millis() - lastSendMs[i] >= 200);

        if (changed || periodic)
        {
            envoyerTrame(i, pos, etat);
            lastPos[i]  = pos;
            lastEtat[i] = etat;
        }
    }
}
/* ============================================================
   Fin de EXSA_Switches.cpp
   ============================================================ */