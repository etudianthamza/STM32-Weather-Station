/*
 * Lecture_girouette.h
 *
 *  Created on: Dec 22, 2025
 *      Author: hamza
 */

#ifndef LECTURE_GIROUETTE_H
#define LECTURE_GIROUETTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================
 *          CONSTANTES
 * ============================ */
#define GIROUETTE_ADC_CHANNEL      ADC_CHANNEL_6    // Canal ADC pour la girouette
#define GIROUETTE_ADC_HANDLE       hadc3            // Handler ADC
#define ADC_RESOLUTION             4095.0f          // Résolution 12 bits
#define VREF_ADC                   3.3f             // Tension de référence

// Seuil pour détecter un changement de direction
#define DIRECTION_CHANGE_THRESHOLD 50               // Points ADC

/* ============================
 *          STRUCTURES
 * ============================ */
typedef struct {
    uint16_t valeur_adc;       // Valeur ADC de référence
    const char* direction;     // Direction textuelle
    const char* direction_court; // Direction abrégée pour l'écran
    float angle_deg;           // Angle en degrés
} DirectionEntry_t;

typedef struct {
    const char* direction_texte;      // Direction complète
    const char* direction_courte;     // Direction courte
    float angle_deg;                  // Angle en degrés
    uint16_t valeur_adc;              // Valeur ADC mesurée
    uint8_t changed;                  // Flag de changement
} GirouetteData_t;

/* ============================
 *          FONCTIONS
 * ============================ */
void Girouette_Init(void);
void Girouette_Start_Conversion(void);
void Girouette_Read_Direction(GirouetteData_t* data);
const char* Girouette_Get_Direction_Text(uint16_t adc_value);
const char* Girouette_Get_Direction_Short(uint16_t adc_value);
float Girouette_Get_Angle(uint16_t adc_value);
uint8_t Girouette_Detect_Change(uint16_t new_value, uint16_t* last_value);

#ifdef __cplusplus
}
#endif

#endif /* LECTURE_GIROUETTE_H */
