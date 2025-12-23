/*
 * screen.c
 *
 *  Created on: Dec 16, 2025
 *      Author: vdela
 */
#include "screen.h"
#include "main.h"
#include "Read_Sensors_THP.h"
#include <string.h>
#include "image.h"
#include "data.h"
#include "Lecture_anenometer.h"
#include "Lecture_girouette.h"
#include "Lecture_pluviometre.h"
/**
 * @brief  Convertit l'angle en direction courte
 * @param  angle : Angle en degrés
 * @return Pointeur vers l'abréviation courte
 */
static const char* angle_to_short_direction(float angle)
{
    // Normaliser l'angle entre 0 et 360
    float normalized_angle = angle;
    while (normalized_angle < 0) normalized_angle += 360.0f;
    while (normalized_angle >= 360.0f) normalized_angle -= 360.0f;

    // Les 16 directions avec leurs plages angulaires
    // Chaque direction couvre 22.5 degrés (360/16 = 22.5)
    // Les seuils sont au milieu entre deux directions

    if (normalized_angle >= 348.75f || normalized_angle < 11.25f)
        return "N";      // Nord
    else if (normalized_angle < 33.75f)
        return "NNE";    // Nord-Nord-Est
    else if (normalized_angle < 56.25f)
        return "NE";     // Nord-Est
    else if (normalized_angle < 78.75f)
        return "ENE";    // Est-Nord-Est
    else if (normalized_angle < 101.25f)
        return "E";      // Est
    else if (normalized_angle < 123.75f)
        return "ESE";    // Est-Sud-Est
    else if (normalized_angle < 146.25f)
        return "SE";     // Sud-Est
    else if (normalized_angle < 168.75f)
        return "SSE";    // Sud-Sud-Est
    else if (normalized_angle < 191.25f)
        return "S";      // Sud
    else if (normalized_angle < 213.75f)
        return "SSW";    // Sud-Sud-Ouest
    else if (normalized_angle < 236.25f)
        return "SW";     // Sud-Ouest
    else if (normalized_angle < 258.75f)
        return "WSW";    // Ouest-Sud-Ouest
    else if (normalized_angle < 281.25f)
        return "W";      // Ouest
    else if (normalized_angle < 303.75f)
        return "WNW";    // Ouest-Nord-Ouest
    else if (normalized_angle < 326.25f)
        return "NW";     // Nord-Ouest
    else if (normalized_angle < 348.75f)
        return "NNW";    // Nord-Nord-Ouest

    return "N/A";  // Ne devrait jamais être atteint
}

void init_screen() {
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, SDRAM_DEVICE_ADDR);

	/* Set text to DEFAULT_FONT */
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

	/* We select the active layer */
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

	/* Clear the Background Layer */
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	/* Set the background color to BLACK */
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

	/* Set text color to GREEN */
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);

	/* Display text on LCD */
	BSP_LCD_DrawBitmap(0, 0, converted_bmp);
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(0, 10, (uint8_t*)"Initialisation", CENTER_MODE);
	BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
}

void update_screen(int current_page)
{
	//Page principale (affichage des données en temps réel)
	if (current_page == 0) {
		// Paramètres visuels
		const uint16_t CARD_WIDTH  = 120;
		const uint16_t CARD_HEIGHT = 80;

		const uint16_t START_X = 20;
		const uint16_t START_Y = 50;
		const uint16_t SPACE_X = 140;
		const uint16_t SPACE_Y = 110;

		const uint32_t CARD_BG_COLOR     = LCD_COLOR_DARKGRAY;
		const uint32_t CARD_BORDER_COLOR = LCD_COLOR_LIGHTGRAY;
		const uint32_t TEXT_COLOR        = LCD_COLOR_WHITE;

		//Données en array de char*
		const char * const measure_names[6] = {
			"Temperature",
			"Pression",
			"Humidite",
			"Vitesse",
			"Direction",
			"Debit de"
		};

		const char * const measure_names2[6] = {
			"",
			"",
			"",
			"du vent",
			"du vent",
			"pluie"
		};

		const char * const measure_units[6] = {
			"C",
			" hPa",
			"%",
			"km/h",
			"",
			"mm/h"
		};

		char measure_values[6][16];

		for (int i = 0; i < 6; i++) {
			strcpy(measure_values[i], "N/A");
		}

		snprintf(measure_values[0], 16, "%.1f", current_data.temperature);
		snprintf(measure_values[1], 16, "%.1f", current_data.pression);
		snprintf(measure_values[2], 16, "%.1f", current_data.humidite);
		snprintf(measure_values[3], 16, "%.1f", current_data.vent_vitesse);
		if (current_data.vent_direction >= 0) {
		    // Utilise la fonction de conversion
		    const char* dir_short = angle_to_short_direction(current_data.vent_direction);
		    snprintf(measure_values[4], 16, "%s", dir_short);  // Juste l'abréviation
		} else {
		    strcpy(measure_values[4], "N/A");
		}
		// Pluie - afficher l'intensité (mm/h)
		snprintf(measure_values[5], 16, "%.1f", current_data.pluie);
		//Affichage

		BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_DisplayStringAt(0, 10, (uint8_t*)"Donnees en temps reel", CENTER_MODE);

		for (int line = 0; line < 2; line++) {
			for (int column = 0; column < 3; column++) {

				int idx = line * 3 + column;
				uint16_t x = START_X + column * SPACE_X;
				uint16_t y = START_Y + line * SPACE_Y;

				//Carte
				BSP_LCD_SetTextColor(CARD_BG_COLOR);
				BSP_LCD_FillRect(x, y, CARD_WIDTH, CARD_HEIGHT);

				BSP_LCD_SetTextColor(CARD_BORDER_COLOR);
				BSP_LCD_DrawRect(x, y, CARD_WIDTH, CARD_HEIGHT);

				BSP_LCD_SetBackColor(CARD_BG_COLOR);
				BSP_LCD_SetTextColor(TEXT_COLOR);

				//Titre de la carte
				BSP_LCD_SetFont(&Font12);
				BSP_LCD_DisplayStringAt(x + 6, y + 6,
										(uint8_t*)measure_names[idx],
										LEFT_MODE);

				BSP_LCD_DisplayStringAt(x + 6, y + 18,
										(uint8_t*)measure_names2[idx],
										LEFT_MODE);

				//Valeur
				BSP_LCD_SetFont(&Font20);
				BSP_LCD_DisplayStringAt(x + 6, y + 42,
										(uint8_t*)measure_values[idx],
										LEFT_MODE);

				//Unité
				BSP_LCD_SetFont(&Font12);
				BSP_LCD_DisplayStringAt(x + CARD_WIDTH - 34, y + 46,
										(uint8_t*)measure_units[idx],
										LEFT_MODE);
			}
		}

		// --- Date & heure (bas droite) ---
		char datetime_str[32];

		snprintf(datetime_str, sizeof(datetime_str),
		         "%02d/%02d/%04d %02d:%02d:%02d",
		         current_data.jour,
		         current_data.mois,
		         current_data.annee,
		         current_data.heure,
		         current_data.minute,
		         current_data.seconde);

		// Style discret
		BSP_LCD_SetFont(&Font12);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);

		// Affichage en bas à droite avec petite marge
		BSP_LCD_DisplayStringAt(
		    BSP_LCD_GetXSize() - 5,
		    BSP_LCD_GetYSize() - 18,
		    (uint8_t*)datetime_str,
		    RIGHT_MODE);
	}
	else
	{
		int GRAPH_X = 20;
		int GRAPH_Y = 60;
		int GRAPH_W = 400;
		int GRAPH_H = 190;
		int LEGEND_X = GRAPH_X + GRAPH_W + 10;

		//Bouton retour
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_FillRect(10, 10, 20, 20);
		BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
		BSP_LCD_DrawRect(10, 10, 20, 20);
		BSP_LCD_SetFont(&Font12);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_DisplayStringAt(16, 14, (uint8_t*)"X", LEFT_MODE);

		// --- Sélection de la donnée ---
		float values[60];

		for (int i = 0; i < 60; i++) {
			switch (current_page) {
			case 1: values[i] = min_data[i].temperature;    break;
			case 2: values[i] = min_data[i].pression;       break;
			case 3: values[i] = min_data[i].humidite;       break;
			case 4: values[i] = min_data[i].vent_vitesse;   break;
			case 5: values[i] = min_data[i].vent_direction; break;
			case 6: values[i] = min_data[i].pluie;          break;
			default: values[i] = 0.0f; break;
			}
		}

		// --- Calcul min / max ---
		float min = values[0];
		float max = values[0];

		for (int i = 1; i < 60; i++) {
			if (values[i] < min) min = values[i];
			if (values[i] > max) max = values[i];
		}

		// Évite division par zéro
		if (max - min < 0.001f) {
			max = min + 1.0f;
		}
		float mid = 0.5f * (min + max);

		// --- Cadre du graphe ---
		BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
		BSP_LCD_FillRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
		BSP_LCD_DrawRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H);
		int y_mid = GRAPH_Y + GRAPH_H - (int)((mid - min) * GRAPH_H / (max - min));

		BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
		BSP_LCD_DrawLine(GRAPH_X, y_mid, GRAPH_X + GRAPH_W, y_mid);

		//légende
		char str_max[16];
		char str_mid[16];
		char str_min[16];

		snprintf(str_max, sizeof(str_max), "%.1f", max);
		snprintf(str_mid, sizeof(str_mid), "%.1f", mid);
		snprintf(str_min, sizeof(str_min), "%.1f", min);

		BSP_LCD_SetFont(&Font12);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

		// MAX (haut)
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_DisplayStringAt(
		    LEGEND_X,
		    GRAPH_Y - 6,
		    (uint8_t*)"Max",
		    LEFT_MODE);

		BSP_LCD_DisplayStringAt(
		    LEGEND_X,
		    GRAPH_Y + 8,
		    (uint8_t*)str_max,
		    LEFT_MODE);

		// MID (centre)
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
		BSP_LCD_DisplayStringAt(
		    LEGEND_X,
		    y_mid - 6,
		    (uint8_t*)str_mid,
		    LEFT_MODE);

		// MIN (bas)
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_DisplayStringAt(
		    LEGEND_X,
		    GRAPH_Y + GRAPH_H - 6,
		    (uint8_t*)"Min",
		    LEFT_MODE);

		BSP_LCD_DisplayStringAt(
		    LEGEND_X,
		    GRAPH_Y + GRAPH_H + 8,
		    (uint8_t*)str_min,
		    LEFT_MODE);

		// --- Tracé du graphe ---
		BSP_LCD_SetTextColor(LCD_COLOR_GREEN);

		for (int i = 1; i < 60; i++) {
			int x1 = GRAPH_X + (GRAPH_W * (i - 1)) / 59;
			int x2 = GRAPH_X + (GRAPH_W * i) / 59;

			int y1 = GRAPH_Y + GRAPH_H
				   - (int)((values[i - 1] - min) * GRAPH_H / (max - min));

			int y2 = GRAPH_Y + GRAPH_H
				   - (int)((values[i] - min) * GRAPH_H / (max - min));

			BSP_LCD_DrawLine(x1, y1, x2, y2);
		}

		// --- Titre ---
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

		const char *title = "";
		switch (current_page) {
		case 1: title = "Temperature (60s)"; break;
		case 2: title = "Pression (60s)"; break;
		case 3: title = "Humidite (60s)"; break;
		case 4: title = "Vitesse du vent (60s)"; break;
		case 5: title = "Direction du vent (60s)"; break;
		case 6: title = "Pluie (60s)"; break;
		}

		BSP_LCD_DisplayStringAt(0, 8, (uint8_t*)title, CENTER_MODE);
	}
}

int handle_touch(int x, int y, int current_page) {
	// Si on n'est PAS sur la page principale,
	// un touch renvoie à la page 0
	if (current_page != 0) {
		if (x > 10 && x <= 30 && y > 10 && y <= 30)
		{
			BSP_LCD_DrawBitmap(0, 0, converted_bmp);
			return 0;
		} else {
			return current_page;
		}
	}

	// Paramètres identiques à update_screen()
	const uint16_t CARD_WIDTH  = 120;
	const uint16_t CARD_HEIGHT = 80;

	const uint16_t START_X = 20;
	const uint16_t START_Y = 50;
	const uint16_t SPACE_X = 140;
	const uint16_t SPACE_Y = 110;

	// Parcours des 6 cartes
	for (int line = 0; line < 2; line++) {
		for (int column = 0; column < 3; column++) {

			uint16_t card_x = START_X + column * SPACE_X;
			uint16_t card_y = START_Y + line * SPACE_Y;

			if (x >= card_x &&
				x <= card_x + CARD_WIDTH &&
				y >= card_y &&
				y <= card_y + CARD_HEIGHT)
			{
				int idx = line * 3 + column;
				BSP_LCD_DrawBitmap(0, 0, converted_bmp);
				return idx + 1; // pages 1 à 6
			}
		}
	}

	// Touch hors des cartes → pas de changement
	return current_page;
}

