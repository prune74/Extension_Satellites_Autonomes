/*
   EXSA_Quadrature.h — Version EXSA universel (architecture FreeRTOS)

   Gestion d’une seule paire de capteurs quadrature (A/B).

   Architecture industrielle :
     - ISR ultra-courte (lecture brute A/B)
     - Transmission des événements via une queue FreeRTOS
     - Traitement quadrature (delta +1 / -1) dans la tâche EXSA
     - Aucun anti-rebond dans l’ISR
     - Aucun appel externe dans l’ISR
     - Aucun calcul dans l’ISR
*/

#pragma once
#include <Arduino.h>
#include <stdint.h>

class EXSA_Quadrature {
public:
    /*
     * initQueue()
     * Initialisation de la queue FreeRTOS
     */
    static void initQueue() noexcept;

    /*
    * installerInterruptions()
    * Installation des interruptions sur les pins A/B
    */
    static void installerInterruptions() noexcept;

    /*
     * lireEvenement()
     * Lecture non bloquante d’un événement A/B dans la queue
     */
    static bool lireEvenement(uint8_t &etat) noexcept;
};

/*
 * isrQuadrature()
 * ISR ultra-courte appelée sur changement d’état des signaux A/B.
 */
void IRAM_ATTR isrQuadrature();
