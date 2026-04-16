/*
📘 FICHE D’ONBOARDING — SA à 2 bits et SA à 3 bits
🎯 Objectif
Comprendre combien de chemins, combien d’aiguilles, et combien de SA un nœud central (SA0) peut gérer selon qu’il utilise 2 bits ou 3 bits.

🟦 1. SA à 2 bits
🔢 Combinaisons possibles
Un codage sur 2 bits donne :
2*2 = 4

 combinaisons
→ 00, 01, 10, 11

🚦 Chemins utilisables
Les 4 combinaisons peuvent être utilisées comme 4 chemins distincts.

Donc :
4 chemins possibles
4 SA atteignables (SA1, SA2, SA3, SA4)

🔧 Nombre d’aiguilles nécessaires
Pour relier 4 états binaires, il faut 3 transitions, donc 3 aiguilles :
transition 00 ↔ 01 → A1
transition 00 ↔ 10 → A2
transition 01 ↔ 11 → A3

Donc :
3 aiguilles max pour un SA à 2 bits

🧠 Résumé SA 2 bits
Élément	Valeur
Bits	2
Combinaisons	4
Chemins utilisables	4
Aiguilles max	3
SA atteignables	4

🟩 2. SA à 3 bits
🔢 Combinaisons possibles
Un codage sur 3 bits donne :
2*2*2 = 8

 combinaisons
→ 000, 001, 010, 011, 100, 101, 110, 111

🚦 Chemins utilisables
Les 8 combinaisons peuvent être utilisées comme 8 chemins distincts.

Donc :
8 chemins possibles
8 SA atteignables (SA1 à SA8)

🔧 Nombre d’aiguilles nécessaires
Pour relier 8 états binaires, il faut 7 transitions, donc 7 aiguilles :
transition 000 ↔ 001 → A1
transition 000 ↔ 010 → A2
transition 000 ↔ 100 → A3
transition 001 ↔ 011 → A4
transition 001 ↔ 101 → A5  
transition 010 ↔ 110 → A6
transition 100 ↔ 110 → A7

Donc :
7 aiguilles max pour un SA à 3 bits

🧠 Résumé SA 3 bits
Élément	Valeur
Bits	3
Combinaisons	8
Chemins utilisables	8
Aiguilles max	7
SA atteignables	8

🟧 3. Symétrie horaire / antihoraire
Autour d’un SA0, on peut appliquer le même modèle :
8 chemins en sens horaire (p000 → p111)
8 chemins en sens antihoraire (m000 → m111)

Donc autour de SA0 :
16 chemins possibles
14 aiguilles max (7 p + 7 m)
16 SA voisins possibles

🟪 4. Résumé global
Modèle	Bits	Combinaisons	Chemins	Aiguilles	SA max
SA 2 bits	2	4	4	3	4
SA 3 bits	3	8	8	7	8

🟦 5. Phrase clé à retenir
Chaque bit supplémentaire double le nombre de chemins et ajoute un niveau complet d’aiguilles.  
2 bits → 3 aiguilles → 4 chemins
3 bits → 7 aiguilles → 8 chemins

*/

/*
🟩 PLAN DE TRAVAIL FINAL (fixé, stable, non‑négociable)

PHASE 1 — EXSA (on le termine entièrement)

Nous allons traiter EXSA dans cet ordre précis :

Étape 1 — Protocole UART partagé (SA_EXSA_Protocol.h)
Un seul fichier
Toutes les trames
Toutes les constantes
Plus aucune divergence
SA et EXSA incluent ce fichier

➡️ Quand cette étape est finie : le langage SA ↔ EXSA est figé.

Étape 2 — EXSA_Config.h (nettoyage + cohérence)
On garde uniquement les pins, PCA9685, aspects
On supprime les trames (elles seront dans le protocole partagé)
On commente proprement
On vérifie les pins (surtout RX/TX)

➡️ Quand cette étape est finie : EXSA a une config propre et claire.

Étape 3 — EXSA_UartRx.h / .cpp
Parsing des trames SA → EXSA
Dispatch vers EXSA_Main
Gestion erreurs
Gestion buffer
Gestion synchro
Aucun code magique

➡️ Quand cette étape est finie : EXSA comprend parfaitement SA.

Étape 4 — EXSA_Main.cpp
Boucle principale
Initialisation PCA9685
Initialisation Charlieplexing
Initialisation UART
Appels vers Signaux / Direction / Servo
Gestion erreurs globales

➡️ Quand cette étape est finie : EXSA fonctionne comme un module autonome.

Étape 5 — EXSA_Signaux.cpp / .h
Application des aspects
Gestion clignotement
Gestion intensités
Gestion œilleton
Gestion mât SNCF

➡️ Quand cette étape est finie : les signaux physiques fonctionnent.

Étape 6 — EXSA_Direction.cpp / .h
Gestion LED directionnelles
Mapping 0..4
Animation éventuelle

➡️ Quand cette étape est finie : les feux directionnels fonctionnent.

Étape 7 — EXSA_Servo.cpp / .h
Application F0/F1/F2
Interpolation
Vitesse
Limites
Test servo
Gestion erreurs servo

➡️ Quand cette étape est finie : les servos fonctionnent.

Étape 8 — Vérification EXSA complète
Tests unitaires
Vérification UART
Vérification PCA9685
Vérification signaux
Vérification servos
Vérification direction
Vérification occupation / ponctuels

➡️ Quand cette étape est finie :

🟢 EXSA est terminé, propre, stable, documenté.

🟦 PHASE 2 — SA (même méthode, même ordre)

Quand EXSA est terminé, on fait exactement la même chose pour SA :
Protocole partagé (déjà fait à l’étape 1)
Config.h
SatTopologieUART
GestionReseau
Discovery
Node
Signaux / Direction / Capteurs
Vérification SA complète

➡️ Quand cette phase est finie :

🟢 SA est terminé, propre, stable, documenté.

🟧 PHASE 3 — Vérification SA ↔ EXSA
Test trames SA → EXSA
Test trames EXSA → SA
Test topologie
Test signaux
Test direction
Test servos
Test occupation
Test ponctuels
Test delta axe
Test cohérence globale

➡️ Quand cette phase est finie :

🟢 SA + EXSA = système complet, cohérent, robuste.
*/

/*
🟢 PHASE 1 — EXSA
✔ Étape 1 — Protocole UART partagé
→ FAIT  
→ Le fichier SA_EXSA_Protocol.h est propre, unique, figé.

✔ Étape 2 — EXSA_Config.h
→ FAIT  
→ Pins propres, PCA9685 propre, aspects propres, RX/TX corrigés.

✔ Étape 3 — EXSA_UartRx
→ FAIT (tu l’avais déjà validé avant cette session)

✔ Étape 4 — EXSA_Main
→ FAIT (optimisé, cohérent, stable)

✔ Étape 5 — EXSA_Signaux
→ FAIT (optimisé, cohérent, LED_MAX, clignotement propre)

✔ Étape 6 — EXSA_Direction
→ FAIT (optimisé, cohérent, PWM propre)

✔ Étape 7 — EXSA_Servo
→ ❗ PAS ENCORE FAIT  
→ C’est la prochaine étape du plan.

✔ Étape 8 — Vérification EXSA complète
→ ❗ À faire après EXSA_Servo
*/
