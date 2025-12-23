/*
 * Lecture_pluviometre.c
 *
 *  Created on: Dec 23, 2025
 *      Author: hamza
 */

#include "Lecture_pluviometre.h"
#include "main.h"
#include "tim.h"
#include <string.h>
#include <stdio.h>

/* ============================
 *     VARIABLES STATIQUES
 * ============================ */
static PluviometreData_t pluviometre_data = {0};
static uint32_t dernier_temps_global_ms = 0;
static uint8_t initialise = 0;

/* ============================
 *     FONCTIONS STATIQUES
 * ============================ */

/**
 * @brief  Met à jour les quantités de pluie à partir des compteurs
 */
static void Update_Pluie_From_Impulsions(void)
{
    // Conversion impulsions → mm
    pluviometre_data.pluie_total_mm = pluviometre_data.total_impulsions * PLUIE_PAR_IMPULSION;
    pluviometre_data.pluie_15min_mm = pluviometre_data.impulsions_15min * PLUIE_PAR_IMPULSION;
    pluviometre_data.pluie_1h_mm = pluviometre_data.impulsions_1h * PLUIE_PAR_IMPULSION;
    pluviometre_data.pluie_24h_mm = pluviometre_data.impulsions_24h * PLUIE_PAR_IMPULSION;

    // Calcul des intensités
    pluviometre_data.intensite_15min_mmh = (pluviometre_data.pluie_15min_mm / 15.0f) * 60.0f; // mm/h
    pluviometre_data.intensite_1h_mmh = pluviometre_data.pluie_1h_mm; // mm/h (car déjà sur 1h)

    // Limiter les valeurs (sécurité)
    if (pluviometre_data.intensite_15min_mmh > INTENSITE_MAX_MMH)
        pluviometre_data.intensite_15min_mmh = INTENSITE_MAX_MMH;
    if (pluviometre_data.intensite_1h_mmh > INTENSITE_MAX_MMH)
        pluviometre_data.intensite_1h_mmh = INTENSITE_MAX_MMH;
}

/**
 * @brief  Gestion anti-rebond (debounce)
 * @return 1 si impulsion valide, 0 si rebond
 */
static uint8_t Debounce_Impulsion(uint32_t timestamp_ms)
{
    static uint32_t dernier_temps_impulsion = 0;

    // Vérifier si assez de temps s'est écoulé depuis la dernière impulsion
    if (timestamp_ms - dernier_temps_impulsion > DEBOUNCE_TIME_MS)
    {
        dernier_temps_impulsion = timestamp_ms;
        return 1; // Impulsion valide
    }

    return 0; // Probablement un rebond, ignorer
}

/* ============================
 *     FONCTIONS PUBLIQUES
 * ============================ */

/**
 * @brief  Initialisation du pluviomètre
 */
void Pluviometre_Init(void)
{
    // Initialisation des données
    memset(&pluviometre_data, 0, sizeof(PluviometreData_t));

    // Initialiser les timestamps
    pluviometre_data.dernier_timestamp_ms = HAL_GetTick();
    dernier_temps_global_ms = HAL_GetTick();

    // Marquer comme initialisé
    initialise = 1;

    printf("Pluviomètre initialisé\r\n");
    printf("  - Facteur: %.4f mm/impulsion\r\n", PLUIE_PAR_IMPULSION);
    printf("  - Débounce: %d ms\r\n", DEBOUNCE_TIME_MS);
}

/**
 * @brief  Gestionnaire d'interruption pour TIM2
 * @note   À appeler depuis HAL_TIM_IC_CaptureCallback
 */
void Pluviometre_Capture_Handler(void)
{
    if (!initialise) return;

    uint32_t timestamp_ms = HAL_GetTick();

    // Vérifier anti-rebond
    if (!Debounce_Impulsion(timestamp_ms))
        return;

    // Incrémenter tous les compteurs
    pluviometre_data.total_impulsions++;
    pluviometre_data.impulsions_15min++;
    pluviometre_data.impulsions_1h++;
    pluviometre_data.impulsions_24h++;

    // Mettre à jour le timestamp
    pluviometre_data.dernier_timestamp_ms = timestamp_ms;

    // Marquer nouvelle mesure
    pluviometre_data.nouvelle_mesure = 1;

    // Mettre à jour les quantités de pluie
    Update_Pluie_From_Impulsions();

    // Debug (optionnel)
    // printf("Pluie: %lu impulsions, Total: %.2f mm\r\n",
    //        pluviometre_data.total_impulsions,
    //        pluviometre_data.pluie_total_mm);
}

/**
 * @brief  Lecture des données du pluviomètre
 * @param  data : Pointeur vers la structure à remplir
 */
void Lecture_pluviometre(volatile PluviometreData_t* data)
{
    if (!initialise || data == NULL)
        return;

    // Copier les données
    memcpy(data, &pluviometre_data, sizeof(PluviometreData_t));

    // Réinitialiser le flag de nouvelle mesure
    pluviometre_data.nouvelle_mesure = 0;
}

/**
 * @brief  Mise à jour des périodes (à appeler périodiquement)
 * @note   Doit être appelé régulièrement (ex: toutes les secondes)
 */
void Pluviometre_Update_Periodes(void)
{
    if (!initialise) return;

    uint32_t temps_actuel_ms = HAL_GetTick();
    uint32_t delta_temps_ms;

    // Gestion du débordement du compteur ms
    if (temps_actuel_ms >= dernier_temps_global_ms)
    {
        delta_temps_ms = temps_actuel_ms - dernier_temps_global_ms;
    }
    else
    {
        // Débordement (après ~49.7 jours)
        delta_temps_ms = (0xFFFFFFFF - dernier_temps_global_ms) + temps_actuel_ms + 1;
    }

    dernier_temps_global_ms = temps_actuel_ms;

    // Accumuler le temps pour chaque période
    pluviometre_data.temps_ecoule_15min += delta_temps_ms;
    pluviometre_data.temps_ecoule_1h += delta_temps_ms;
    pluviometre_data.temps_ecoule_24h += delta_temps_ms;

    // Vérifier période de 15 minutes
    if (pluviometre_data.temps_ecoule_15min >= PERIODE_15MIN_MS)
    {
        // Afficher résumé (optionnel)
        printf("[Pluie 15min] Impulsions: %lu | Pluie: %.2f mm | Intensité: %.2f mm/h\r\n",
               pluviometre_data.impulsions_15min,
               pluviometre_data.pluie_15min_mm,
               pluviometre_data.intensite_15min_mmh);

        // Réinitialiser période 15min
        pluviometre_data.impulsions_15min = 0;
        pluviometre_data.temps_ecoule_15min = 0;
    }

    // Vérifier période de 1 heure
    if (pluviometre_data.temps_ecoule_1h >= PERIODE_1H_MS)
    {
        // Afficher résumé (optionnel)
        printf("[Pluie 1h] Impulsions: %lu | Pluie: %.2f mm\r\n",
               pluviometre_data.impulsions_1h,
               pluviometre_data.pluie_1h_mm);

        // Réinitialiser période 1h
        pluviometre_data.impulsions_1h = 0;
        pluviometre_data.temps_ecoule_1h = 0;
    }

    // Vérifier période de 24 heures
    if (pluviometre_data.temps_ecoule_24h >= PERIODE_24H_MS)
    {
        // Afficher résumé (optionnel)
        printf("[Pluie 24h] Impulsions: %lu | Pluie: %.2f mm\r\n",
               pluviometre_data.impulsions_24h,
               pluviometre_data.pluie_24h_mm);

        // Réinitialiser période 24h
        pluviometre_data.impulsions_24h = 0;
        pluviometre_data.temps_ecoule_24h = 0;
    }

    // Mettre à jour les quantités (au cas où)
    Update_Pluie_From_Impulsions();
}

/**
 * @brief  Réinitialise le compteur total
 */
void Pluviometre_Reset_Total(void)
{
    pluviometre_data.total_impulsions = 0;
    pluviometre_data.pluie_total_mm = 0.0f;
    printf("Pluviomètre: compteur total réinitialisé\r\n");
}

/**
 * @brief  Réinitialise la période de 15 minutes
 */
void Pluviometre_Reset_15min(void)
{
    pluviometre_data.impulsions_15min = 0;
    pluviometre_data.temps_ecoule_15min = 0;
    printf("Pluviomètre: période 15min réinitialisée\r\n");
}

/**
 * @brief  Réinitialise la période de 1 heure
 */
void Pluviometre_Reset_1h(void)
{
    pluviometre_data.impulsions_1h = 0;
    pluviometre_data.temps_ecoule_1h = 0;
    printf("Pluviomètre: période 1h réinitialisée\r\n");
}

/**
 * @brief  Réinitialise la période de 24 heures
 */
void Pluviometre_Reset_24h(void)
{
    pluviometre_data.impulsions_24h = 0;
    pluviometre_data.temps_ecoule_24h = 0;
    printf("Pluviomètre: période 24h réinitialisée\r\n");
}

/**
 * @brief  Calcule l'intensité sur 15 minutes
 * @return Intensité en mm/h
 */
float Pluviometre_Calculer_Intensite_15min(void)
{
    if (pluviometre_data.temps_ecoule_15min == 0)
        return 0.0f;

    // Pluie tombée / temps écoulé * 3600 (pour 1 heure)
    float temps_h = pluviometre_data.temps_ecoule_15min / 3600000.0f; // ms → heures
    if (temps_h < 0.001f) return 0.0f;

    return (pluviometre_data.pluie_15min_mm / temps_h);
}

/**
 * @brief  Calcule l'intensité sur 1 heure
 * @return Intensité en mm/h
 */
float Pluviometre_Calculer_Intensite_1h(void)
{
    if (pluviometre_data.temps_ecoule_1h == 0)
        return 0.0f;

    float temps_h = pluviometre_data.temps_ecoule_1h / 3600000.0f;
    if (temps_h < 0.001f) return 0.0f;

    return (pluviometre_data.pluie_1h_mm / temps_h);
}

/**
 * @brief  Retourne le nombre total d'impulsions (debug)
 * @return Nombre total d'impulsions
 */
uint32_t Pluviometre_Get_Total_Impulsions(void)
{
    return pluviometre_data.total_impulsions;
}
