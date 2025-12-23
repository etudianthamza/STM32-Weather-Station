/*
 * Lecture_anenometer.c
 *
 *  Created on: Dec 13, 2025
 *      Author: hamza
 */

#include "Lecture_anenometer.h"
#include "tim.h"
#include "stm32f7xx_hal_rcc.h"
#include <math.h>

/* ============================
 *          EXTERN
 * ============================ */
extern volatile uint8_t Flag_Tim1;  // Interruption TIM2 (capture)

/* ============================
 *          DEFINES
 * ============================ */
#define RADIUS_M           0.07f      // Rayon de l'anémomètre (7cm)
#define MPS_TO_KMH         3.6f       // Conversion m/s → km/h
#define IMPULSES_PER_REV   2.0f       // 2 impulsions par tour (front montant et descendant)
#define MIN_FREQ_HZ        0.1f       // Fréquence minimale détectable (0.1 Hz)
#define MAX_FREQ_HZ        125.0f     // Fréquence maximale (125 Hz)
#define TIM1_CLOCK_MHZ     100.0f     // Fréquence d'horloge de TIM2 en MHz (à ajuster)

/* ============================
 *     STATIC VARIABLES
 * ============================ */
static uint32_t last_capture = 0;     // Dernière valeur capturée
static uint8_t  first_capture = 0;    // Première capture effectuée ?
static float    wind_speed_kmh = 0.0f; // Vitesse filtrée (km/h)
static uint32_t capture_time = 0;     // Temps de la dernière capture (ms)
static uint32_t pulse_count = 0;      // Nombre d'impulsions (pour debug)

/* ============================
 *     STATIC FUNCTIONS
 * ============================ */

/**
 * @brief  Calcule la durée d'une impulsion en microsecondes
 * @param  capture1 : Première valeur de capture
 * @param  capture2 : Deuxième valeur de capture
 * @return Période en microsecondes
 */
static float Calculate_Period_us(uint32_t capture1, uint32_t capture2)
{
    uint32_t delta_ticks;

    // Gestion de l'overflow du timer (16 bits)
    if (capture2 >= capture1) {
        delta_ticks = capture2 - capture1;
    } else {
        // Overflow : (max_value - capture1) + capture2 + 1
        delta_ticks = (0xFFFF - capture1) + capture2 + 1;
    }

    // Convertir les ticks en temps
    // Période (µs) = delta_ticks × (1 / freq_timer) × 1e6
    // Avec freq_timer = TIM2_CLOCK_MHZ / (PSC + 1)
    float timer_freq_mhz = TIM1_CLOCK_MHZ / (htim1.Init.Prescaler + 1);
    float period_us = delta_ticks / timer_freq_mhz;

    return period_us;
}

/**
 * @brief  Calcule la vitesse à partir de la période mesurée
 * @param  period_us : Période en microsecondes
 * @return Vitesse en km/h
 */
static float Calculate_Speed_From_Period(float period_us)
{
    // 1. Période → Fréquence (Hz)
    if (period_us <= 0) return 0.0f;
    float freq_hz = 1000000.0f / period_us;  // 1e6 / période(µs)

    // 2. Fréquence → Tours par seconde
    float rps = freq_hz / IMPULSES_PER_REV;  // Tours par seconde

    // 3. Vitesse linéaire (m/s) = Circonférence × tours/seconde
    float speed_mps = 2.0f * M_PI * RADIUS_M * rps;

    // 4. Conversion en km/h
    return speed_mps * MPS_TO_KMH;
}

/* ============================
 *     PUBLIC FUNCTIONS
 * ============================ */

/**
 * @brief  Gestionnaire d'interruption pour TIM2
 * @note   Appelé par HAL_TIM_IC_CaptureCallback
 */
void Anemometer_Capture_Handler(void)
{
    // Incrémenter le compteur d'impulsions (debug)
    pulse_count++;

    // Marquer qu'une nouvelle capture est disponible
    // Le calcul sera fait dans Lecture_anenometer()
}

/**
 * @brief  Lecture de la vitesse du vent
 * @param  p_wind_speed : Pointeur vers la variable de vitesse
 * @note   Doit être appelé quand Flag_Tim2 est à 1
 */
void Lecture_anenometer(float *p_wind_speed)
{
    // Si pas de nouvelle capture, retourner la dernière valeur
    if (!Flag_Tim1) {
        *p_wind_speed = wind_speed_kmh;
        return;
    }

    Flag_Tim1 = 0;  // Réinitialiser le flag

    // 1. Lire la valeur actuelle du registre de capture
    uint32_t current_capture = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1);

    // 2. Pour la première capture, on initialise seulement
    if (!first_capture) {
        last_capture = current_capture;
        first_capture = 1;
        capture_time = HAL_GetTick();
        *p_wind_speed = 0.0f;
        return;
    }

    // 3. Calculer la période entre les deux dernières captures
    float period_us = Calculate_Period_us(last_capture, current_capture);

    // Debug
    // printf("Period: %.1f us, Capture: %lu -> %lu\n",
    //        period_us, last_capture, current_capture);

    // 4. Mettre à jour pour la prochaine mesure
    last_capture = current_capture;

    // 5. Vérifier si la période est dans une plage raisonnable
    //    Correspond à des fréquences entre ~0.33Hz et 125Hz
    if (period_us < 8000.0f || period_us > 3000000.0f) {
        // Période hors plage (trop courte ou trop longue)
        *p_wind_speed = wind_speed_kmh;
        return;
    }

    // 6. Calculer la vitesse à partir de la période
    float new_speed_kmh = Calculate_Speed_From_Period(period_us);

    // 7. Appliquer un filtre exponentiel
    if (wind_speed_kmh == 0.0f) {
        // Première mesure valide
        wind_speed_kmh = new_speed_kmh;
    } else {
        // Filtre : 70% ancienne valeur + 30% nouvelle valeur
        wind_speed_kmh = 0.7f * wind_speed_kmh + 0.3f * new_speed_kmh;
    }

    // 8. Limiter la valeur (sécurité)
    if (wind_speed_kmh < 0.1f) wind_speed_kmh = 0.0f;
    if (wind_speed_kmh > 200.0f) wind_speed_kmh = 200.0f;

    // 9. Retourner la valeur
    *p_wind_speed = wind_speed_kmh;

    // Debug
    // printf("Speed: %.2f km/h (Period: %.0f us, Pulses: %lu)\n",
    //        wind_speed_kmh, period_us, pulse_count);
}

/**
 * @brief  Initialisation de l'anémomètre
 */
void Anemometer_Init(void)
{
    last_capture = 0;
    first_capture = 0;
    wind_speed_kmh = 0.0f;
    pulse_count = 0;
    capture_time = HAL_GetTick();
}

/**
 * @brief  Obtient le nombre d'impulsions (debug)
 * @return Nombre total d'impulsions détectées
 */
uint32_t Anemometer_Get_Pulse_Count(void)
{
    return pulse_count;
}

/**
 * @brief  Réinitialise le compteur d'impulsions
 */
void Anemometer_Reset_Pulse_Count(void)
{
    pulse_count = 0;
}
