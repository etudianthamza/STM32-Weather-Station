/*
 * screen.h
 *
 *  Created on: Dec 16, 2025
 *      Author: vdela
 */

#ifndef INC_SCREEN_H_
#define INC_SCREEN_H_

#include "stm32746g_discovery.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include <stdio.h>
#include "data.h"
#include "main.h"

extern volatile float vitesse_vent;
extern volatile meteo_data_t current_data;
extern volatile meteo_data_t min_data[60];

void init_screen();
void update_screen(int current_page);
int handle_touch(int x, int y, int current_page);


#endif /* INC_SCREEN_H_ */
