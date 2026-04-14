#pragma once
#include <Arduino.h>
#include "EXSA_Multiplexeur.h"
#include "EXSA_Config.h"
#include "SA_EXSA_Protocol.h"

/*
 * EXSA_Signaux.h — Version ENUM (Option A)
 * ------------------------------------------------------------
 * Gestion des aspects SNCF pour un mât de signalisation.
 *
 * Ce module :
 *   - reçoit un aspect SNCF sous forme d’un enum ExsaAspect
 *   - applique les capacités du signal (manœuvre ou principal)
 *   - décide quelles LED doivent être allumées
 *   - applique intensité + clignotement
 *   - délègue l’affichage réel au multiplexeur Charlieplexing
 *
 * Il ne pilote PAS directement les GPIO :
 *   → EXSA_Multiplexeur gère le scan et le PWM.
 */

/*-------------------------------------------------------------
   Enum des aspects SNCF (1 octet envoyé SA → EXSA)
--------------------------------------------------------------*/
/*enum ExsaAspect : uint8_t {
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
*/
class EXSA_Signaux {
public:

    /*
     * Constructeur
     * ---------------------------------------------------------
     * mux          : pointeur vers le multiplexeur Charlieplexing
     * estManoeuvre : true si le signal est un signal de manœuvre
     *                (violet au carré), false sinon (rouge)
     */
    explicit EXSA_Signaux(EXSA_Multiplexeur* mux, bool estManoeuvre);

    /*
     * setAspect()
     * ---------------------------------------------------------
     * Reçoit un aspect SNCF sous forme d’un enum ExsaAspect.
     *
     * Fonctionnement :
     *   - compare avec l’aspect actuel
     *   - si identique → ne fait rien (return false)
     *   - sinon :
     *       • éteint toutes les LED
     *       • sélectionne les LED selon l’aspect
     *       • active éventuellement le clignotement
     *
     * Retourne true si l’aspect a changé, false sinon.
     */
    [[nodiscard]] bool setAspect(ExsaAspect aspect);

    /*
     * update()
     * ---------------------------------------------------------
     * Met à jour le multiplexeur :
     *   - clignotement (toggle ON/OFF)
     *   - scan Charlieplexing (1 LED par cycle)
     *
     * À appeler régulièrement dans EXSA_Main::loop().
     */
    void update();

private:
    EXSA_Multiplexeur* mux;   // Multiplexeur Charlieplexing associé au mât
    bool estManoeuvre;        // Signal principal (rouge) ou de manœuvre (violet)
    ExsaAspect aspectActuel;  // Aspect actuellement affiché (enum)
};

/* ============================================================
   Fin de EXSA_Signaux.h — Version ENUM
   ============================================================ */
