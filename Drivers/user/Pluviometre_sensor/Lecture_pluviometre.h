/*
 * Lecture_pluviometre.h
 *
 *  Created on: Dec 23, 2025
 *      Author: hamza
 */


#ifndef LECTURE_PLUVIOMETRE_H
#define LECTURE_PLUVIOMETRE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================
 *          CONSTANTES
 * ============================ */
#define PLUIE_PAR_IMPULSION      0.2794f    // 0.2794 mm par impulsion (à vérifier selon modèle)
#define IMPULSIONS_PAR_MM        3.579f     // 1 / 0.2794 (pour conversion inverse)

// Périodes en millisecondes
#define PERIODE_15MIN_MS         (15 * 60 * 1000)   // 15 minutes
#define PERIODE_1H_MS            (60 * 60 * 1000)   // 1 heure
#define PERIODE_24H_MS           (24 * 60 * 60 * 1000) // 24 heures

// Seuils de détection
#define DEBOUNCE_TIME_MS         50         // Anti-rebond en ms
#define INTENSITE_MAX_MMH        200.0f     // Intensité pluie max (mm/h)

/* ============================
 *          STRUCTURES
 * ============================ */
typedef struct {
    // Compteurs d'impulsions
    uint32_t total_impulsions;      // Depuis le démarrage
    uint32_t impulsions_15min;      // Dernières 15 minutes
    uint32_t impulsions_1h;         // Dernière heure
    uint32_t impulsions_24h;        // Dernières 24 heures

    // Quantités de pluie (mm)
    float pluie_total_mm;           // Total depuis démarrage
    float pluie_15min_mm;           // Dernières 15 minutes
    float pluie_1h_mm;              // Dernière heure
    float pluie_24h_mm;             // Dernières 24 heures

    // Intensités (mm/h)
    float intensite_15min_mmh;      // Intensité sur 15 min
    float intensite_1h_mmh;         // Intensité sur 1 heure

    // Temps et périodes
    uint32_t dernier_timestamp_ms;  // Dernière impulsion
    uint32_t temps_ecoule_15min;    // Temps écoulé pour période 15min
    uint32_t temps_ecoule_1h;       // Temps écoulé pour période 1h
    uint32_t temps_ecoule_24h;      // Temps écoulé pour période 24h

    uint8_t nouvelle_mesure;        // Flag nouvelle mesure
    uint8_t en_erreur;              // Flag erreur détection
} PluviometreData_t;

/* ============================
 *          FONCTIONS
 * ============================ */
// Initialisation
void Pluviometre_Init(void);

// Gestionnaire d'interruption (appelé par callback TIM2)
void Pluviometre_Capture_Handler(void);

// Lecture des données courantes
void Lecture_pluviometre(volatile PluviometreData_t* data);

// Mise à jour des périodes (à appeler périodiquement, ex: toutes les secondes)
void Pluviometre_Update_Periodes(void);

// Réinitialisation des compteurs
void Pluviometre_Reset_Total(void);
void Pluviometre_Reset_15min(void);
void Pluviometre_Reset_1h(void);
void Pluviometre_Reset_24h(void);

// Fonctions de calcul
float Pluviometre_Calculer_Intensite_15min(void);
float Pluviometre_Calculer_Intensite_1h(void);

// Debug
uint32_t Pluviometre_Get_Total_Impulsions(void);

#ifdef __cplusplus
}
#endif

#endif /* LECTURE_PLUVIOMETRE_H */
