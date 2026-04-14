#include "EXSA_Signaux.h"
#include "EXSA_Config.h"

/*-------------------------------------------------------------
   Constructeur
--------------------------------------------------------------*/
EXSA_Signaux::EXSA_Signaux(EXSA_Multiplexeur* m, bool manoeuvre)
    : mux(m),
      estManoeuvre(manoeuvre),
      aspectActuel(ASPECT_DEFAUT)
{
}

/*-------------------------------------------------------------
   setAspect() — Version ENUM
   Reçoit un aspect simple (ExsaAspect)
   Applique directement les LED correspondantes.
--------------------------------------------------------------*/
bool EXSA_Signaux::setAspect(ExsaAspect aspect)
{
    if (aspect == aspectActuel)
        return false;

    aspectActuel = aspect;

    // 1) Tout éteindre
    for (uint8_t i = 0; i < LED_MAX; i++)
        mux->reglerLed(static_cast<ExsaLedId>(i), false);

    // 2) Sélection des LED selon l’aspect
    switch (aspect)
    {
        case ASPECT_CARRE:
            if (estManoeuvre) {
                // Carré violet
                mux->reglerLed(LED_CARRE_VIOLET, true);
            } else {
                // Carré rouge (2 rouges)
                mux->reglerLed(LED_SEMAPHORE, true);
                mux->reglerLed(LED_CARRE, true);
            }
            break;

        case ASPECT_SEMAPHORE:
            mux->reglerLed(LED_SEMAPHORE, true);
            mux->reglerLed(LED_OEILLETON, true);
            break;

        case ASPECT_AVERTISSEMENT:
            mux->reglerLed(LED_AVERTISSEMENT, true);
            break;

        case ASPECT_RALENTISSEMENT_30:
            mux->reglerLed(LED_RALENTISSEMENT, true);
            break;

        case ASPECT_RALENTISSEMENT_60:
            mux->reglerLed(LED_RALENTISSEMENT, true, 0, true); // clignotant
            break;

        case ASPECT_RAPPEL_30:
            mux->reglerLed(LED_RAPPEL, true);
            break;

        case ASPECT_RAPPEL_60:
            mux->reglerLed(LED_RAPPEL, true, 0, true); // clignotant
            break;

        case ASPECT_VOIE_LIBRE:
            mux->reglerLed(LED_LIBRE, true);
            break;

        case ASPECT_MANOEUVRE:
            mux->reglerLed(LED_MANOEUVRE, true);
            break;

        case ASPECT_MASQUE:
            // tout OFF (déjà fait)
            break;

        case ASPECT_DEFAUT:
        default:
            mux->reglerLed(LED_OEILLETON, true, 0, true); // clignotant
            break;
    }

    return true;
}

/*-------------------------------------------------------------
   update()
--------------------------------------------------------------*/
void EXSA_Signaux::update()
{
    mux->mettreAJour();
}
