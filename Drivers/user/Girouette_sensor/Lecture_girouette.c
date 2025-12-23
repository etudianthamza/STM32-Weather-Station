/*
 * Lecture_girouette.c
 *
 *  Created on: Dec 22, 2025
 *      Author: hamza
 */

#include "Lecture_girouette.h"
#include "main.h"
#include "adc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================
 *     TABLE DE CORRESPONDANCE
 * ============================ */
static const DirectionEntry_t direction_table[] = {
    {350,  "112.5° ESE (Est-Sud-Est)",      "ESE", 112.5f},
    {400,  "67.5° ENE (Est-Nord-Est)",      "ENE", 67.5f},
    {450,  "90° E (Est)",                   "E",   90.0f},
    {680,  "157.5° SSE (Sud-Sud-Est)",      "SSE", 157.5f},
    {990,  "135° SE (Sud-Est)",             "SE",  135.0f},
    {1350, "202.5° SSW (Sud-Sud-Ouest)",    "SSW", 202.5f},
    {1600, "180° S (Sud)",                  "S",   180.0f},
    {2150, "22.5° NNE (Nord-Nord-Est)",     "NNE", 22.5f},
    {2400, "45° NE (Nord-Est)",             "NE",  45.0f},
    {3000, "247.5° WSW (Ouest-Sud-Ouest)",  "WSW", 247.5f},
    {3100, "225° SW (Sud-Ouest)",           "SW",  225.0f},
    {3400, "337.5° NNW (Nord-Nord-Ouest)",  "NNW", 337.5f},
    {3700, "0° N (Nord)",                   "N",   0.0f},
    {3850, "292.5° WNW (Ouest-Nord-Ouest)", "WNW", 292.5f},
    {4000, "315° NW (Nord-Ouest)",          "NW",  315.0f},
    {4050, "270° W (Ouest)",                "W",   270.0f}
};

static const uint8_t NUM_DIRECTIONS = sizeof(direction_table) / sizeof(direction_table[0]);

/* ============================
 *     VARIABLES STATIQUES
 * ============================ */
static uint16_t last_adc_value = 0;
static uint8_t first_measure = 1;
static GirouetteData_t girouette_data = {0};

/* ============================
 *     FONCTIONS STATIQUES
 * ============================ */

/**
 * @brief  Recherche binaire dans la table des directions
 * @param  adc_value : Valeur ADC à rechercher
 * @return Index dans la table ou -1 si non trouvé
 */
static int8_t Binary_Search_Direction(uint16_t adc_value)
{
    // Recherche linéaire simple (table petite) - pourrait être optimisée
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        uint16_t seuil;

        // Calcul du seuil (milieu entre la valeur courante et la suivante)
        if (i + 1 < NUM_DIRECTIONS) {
            seuil = (direction_table[i].valeur_adc + direction_table[i + 1].valeur_adc) / 2;
        } else {
            seuil = 4095; // Dernier élément
        }

        if (adc_value <= seuil) {
            return i;
        }
    }

    return -1; // Non trouvé
}

/**
 * @brief  Interpolation linéaire pour obtenir un angle précis
 * @param  adc_value : Valeur ADC mesurée
 * @param  idx : Index dans la table
 * @return Angle interpolé en degrés
 */
static float Interpolate_Angle(uint16_t adc_value, int idx)
{
    if (idx < 0 || idx >= NUM_DIRECTIONS) {
        return 0.0f;
    }

    // Si c'est le dernier élément, retourner l'angle directement
    if (idx == NUM_DIRECTIONS - 1) {
        return direction_table[idx].angle_deg;
    }

    // Interpolation linéaire entre deux points
    uint16_t adc_low = direction_table[idx].valeur_adc;
    uint16_t adc_high = direction_table[idx + 1].valeur_adc;
    float angle_low = direction_table[idx].angle_deg;
    float angle_high = direction_table[idx + 1].angle_deg;

    // Éviter la division par zéro
    if (adc_high == adc_low) {
        return angle_low;
    }

    // Formule d'interpolation : y = y0 + (x - x0) * (y1 - y0) / (x1 - x0)
    return angle_low + ((float)(adc_value - adc_low) * (angle_high - angle_low) / (float)(adc_high - adc_low));
}

/* ============================
 *     FONCTIONS PUBLIQUES
 * ============================ */

/**
 * @brief  Initialisation de la girouette
 */
void Girouette_Init(void)
{
    last_adc_value = 0;
    first_measure = 1;

    // Initialisation des données
    girouette_data.direction_texte = "Initialisation";
    girouette_data.direction_courte = "INIT";
    girouette_data.angle_deg = 0.0f;
    girouette_data.valeur_adc = 0;
    girouette_data.changed = 0;

    printf("Girouette initialisée (%d directions dans la table)\r\n", NUM_DIRECTIONS);
}

/**
 * @brief  Démarre une conversion ADC
 */
void Girouette_Start_Conversion(void)
{
    // Démarrer la conversion ADC
    if (HAL_ADC_Start(&GIROUETTE_ADC_HANDLE) != HAL_OK) {
        printf("Erreur démarrage ADC girouette\r\n");
    }
}

/**
 * @brief  Lecture et traitement de la direction
 * @param  data : Pointeur vers la structure de données à remplir
 */
void Girouette_Read_Direction(volatile GirouetteData_t* data)
{
    uint16_t adc_value = 0;
    ADC_HandleTypeDef* hadc = &hadc3;  // Utilise directement hadc3

    // 1. Configuration de l'ADC pour une seule conversion
    hadc->Instance->CR2 &= ~ADC_CR2_CONT;  // Mode single, pas continu

    // 2. Démarrer la conversion
    if (HAL_ADC_Start(hadc) != HAL_OK) {
        printf("Erreur démarrage ADC girouette\r\n");
        return;
    }

    // 3. Attendre la fin de la conversion
    if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(hadc);
    } else {
        printf("Timeout ADC girouette\r\n");
        adc_value = 0;
    }

    // 4. Arrêter l'ADC
    HAL_ADC_Stop(hadc);

    // 5. Détecter un changement significatif
    static uint16_t last_valid_adc = 0;

    if (first_measure) {
        girouette_data.changed = 1;
        first_measure = 0;
    } else {
        // Utiliser une fonction simplifiée de détection
        if (adc_value > 0 && abs((int)adc_value - (int)last_valid_adc) > DIRECTION_CHANGE_THRESHOLD) {
            girouette_data.changed = 1;
        } else {
            girouette_data.changed = 0;
        }
    }

    last_valid_adc = adc_value;

    // 6. Obtenir la direction (version robuste)
    if (adc_value < 100) {  // Valeur trop basse
        girouette_data.direction_texte = "Signal faible/Erreur";
        girouette_data.direction_courte = "ERR";
        girouette_data.angle_deg = -1.0f;
        girouette_data.valeur_adc = adc_value;
    } else {
        int8_t idx = Binary_Search_Direction(adc_value);

        if (idx >= 0) {
            girouette_data.direction_texte = direction_table[idx].direction;
            girouette_data.direction_courte = direction_table[idx].direction_court;
            girouette_data.angle_deg = direction_table[idx].angle_deg;  // Version simple d'abord
            girouette_data.valeur_adc = adc_value;

        } else {
            girouette_data.direction_texte = "Direction hors plage";
            girouette_data.direction_courte = "N/A";
            girouette_data.angle_deg = -1.0f;
            girouette_data.valeur_adc = adc_value;
        }
    }

    // 7. Copier les données
    if (data != NULL) {
        memcpy(data, &girouette_data, sizeof(GirouetteData_t));
    }
}

/**
 * @brief  Obtient le texte de direction à partir d'une valeur ADC
 * @param  adc_value : Valeur ADC
 * @return Pointeur vers la chaîne de direction
 */
const char* Girouette_Get_Direction_Text(uint16_t adc_value)
{
    int8_t idx = Binary_Search_Direction(adc_value);

    if (idx >= 0) {
        return direction_table[idx].direction;
    }

    return "Direction inconnue";
}

/**
 * @brief  Obtient la direction courte (pour l'écran)
 * @param  adc_value : Valeur ADC
 * @return Pointeur vers la chaîne de direction courte
 */
const char* Girouette_Get_Direction_Short(uint16_t adc_value)
{
    int8_t idx = Binary_Search_Direction(adc_value);

    if (idx >= 0) {
        return direction_table[idx].direction_court;
    }

    return "N/A";
}

/**
 * @brief  Obtient l'angle en degrés
 * @param  adc_value : Valeur ADC
 * @return Angle en degrés
 */
float Girouette_Get_Angle(uint16_t adc_value)
{
    int8_t idx = Binary_Search_Direction(adc_value);

    if (idx >= 0) {
        return Interpolate_Angle(adc_value, idx);
    }

    return 0.0f;
}

/**
 * @brief  Détecte un changement significatif de direction
 * @param  new_value : Nouvelle valeur ADC
 * @param  last_value : Pointeur vers l'ancienne valeur (sera mise à jour)
 * @return 1 si changement détecté, 0 sinon
 */
uint8_t Girouette_Detect_Change(uint16_t new_value, uint16_t* last_value)
{
    if (*last_value == 0) {
        return 1; // Première mesure
    }

    int16_t difference = abs((int16_t)new_value - (int16_t)*last_value);

    // Mettre à jour la dernière valeur
    *last_value = new_value;

    // Changement si différence > seuil
    return (difference > DIRECTION_CHANGE_THRESHOLD) ? 1 : 0;
}
