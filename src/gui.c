#include "../lib/observatory.h"

void start_ui(){
	int c;
	int i, k;
	char s[12];
	char nl[16];
	char ml[16];
	int x, y;

	clear_to_color(screen, BGC);

	do{
		if (keypressed()) {
			c = readkey();
			k = c >> 8;
		}

		for(i = 0; i < N; i++){
			textout_centre_ex(screen, font, "SPACE to begin the observation",
						 XWIN / 2, YWIN / 2, 14, 0);
			line(screen, 0, YWIN - LINE, XWIN, YWIN - LINE, 14);

			x = BORDER + i * XWIN/N;
			y = YWIN - LINE + BORDER;
			sprintf(s, "Telescope %d", i+1);
			textout_ex(screen, font, s, x, y, 15, 0);

			x += BORDER;
			y += BORDER;
			sprintf(nl, "noise level: %d", tel.noise_level[i]);
			textout_ex(screen, font, nl, x, y, 15, 0);

			y += BORDER;
			sprintf(ml, "motor level: %d", tel.motor_level[i]);
			textout_ex(screen, font, ml, x, y, 15, 0);


			line(screen, (i+1) * XWIN/N, YWIN - LINE, (i+1) * XWIN/N, YWIN, 15);
		}
			
	}while(k != KEY_SPACE);

	clear_to_color(screen, BGC);
	
	line(screen, 0, YWIN - BASE, XWIN, YWIN - BASE, 14); 
}

void gui_planet(BITMAP *buffer){
	int x, y;

	if(planet_x < XDIAG + OBS_SHAPE/2){
			x = planet_x - OBS_SHAPE/2;
			y = planet_y - OBS_SHAPE/2;
			stretch_sprite(buffer, planet_img, x, y, 300, 300);
	}

	// Realizzo il cielo
	clear_to_color(sky, BGC);
	stretch_sprite(sky, planet_img, x, y, 300, 300);
}

void gui_telescopes(BITMAP *buffer){
	int i;
	int x1, y1;
	int x2, y2;

	for(i = 0; i < N; i++){
			x1 = 1 + i * (BASE - BORDER);
			y1 = YWIN - LINE + BORDER;
			x2 = (BASE - BORDER) * (i + 1); 
			y2 = y1 + BASE - BORDER - 1;

			textout_centre_ex(buffer, font, "Telescope", (x1 + x2) / 2,
								YWIN - LINE + 2, 100, 0);

			stretch_sprite(buffer, tel.observation[i], x1, y1, x2 - x1, y2 - y1);

			rect(buffer, tel.x_pred[i], tel.y_pred[i], tel.x_pred[i] + 2, tel.y_pred[i] + 2, 40);

			rect(buffer, x1, y1, x2, y2, 100);

			if((mouse_x > x1) && (mouse_x < x2)){
				if((mouse_y > y1) && (mouse_y < y2)){
					// Scrivo observation window sul buffer.
					rect(buffer, tel.x_obs[i] - OBS_SHAPE/2,
					 tel.y_obs[i] - OBS_SHAPE/2, tel.x_obs[i] + OBS_SHAPE/2,
					  tel.y_obs[i] + OBS_SHAPE/2, 14);
					line(buffer, tel.x_tel[i], tel.y_tel[i], tel.x_obs[i],
					 tel.y_obs[i], 14);
					line(buffer, tel.x_tel[i], tel.y_tel[i], tel.x_pred[i],
					 tel.y_pred[i], 30);
				}
			}
		}

		if(tel.elaborated == 1){
			x1 = 1 + N * (BASE - BORDER);
			y1 = YWIN - LINE + BORDER;
			x2 = (BASE - BORDER) * (N + 1); 
			y2 = y1 + BASE - BORDER - 1;
			textout_centre_ex(buffer, font, "Result:", (x1 + x2) / 2,
			 YWIN - LINE + 2, 14, 0);
			stretch_sprite(buffer, result, x1, y1, x2 - x1, y2 - y1);
			rect(buffer, x1 + 1, y1, x2, y2, 14);
		}
}

void gui_param_interface(BITMAP *buffer){
	int i;
	char s[12];
	char nl[17];
	char ml[17];

	rectfill(buffer, XDIAG, YDIAG, XWIN, YDIAG+HDIAG, BGC);
	rect(buffer, XDIAG, YDIAG, XWIN - 1, YDIAG + HDIAG, 15);

		for(i = 0; i < N; i++){
			sprintf(s, "Telescope %d", i+1);
			textout_ex(buffer, font, s, XDIAG + BORDER,
			 YDIAG + TBLOCK*(i) + BORDER, 15, 0);

			sprintf(nl, "noise level: %d%%", tel.noise_level[i] / NOISE_VAL_MULTIPLIER);
			textout_ex(buffer, font, nl, XDIAG + BORDER*2,
			 YDIAG + TBLOCK*(i) + BORDER*2, 15, 0);

			sprintf(nl, "motor level: %d%%", tel.motor_level[i] * 10);
			textout_ex(buffer, font, nl, XDIAG + BORDER*2,
			 YDIAG + TBLOCK*(i) + BORDER*3, 15, 0);
		}
}

void gui(){
	int i;
	int x, y;
	int x1, y1;
	int x2, y2;
	BITMAP *buffer;
	char s[12];
	char nl[16];

	buffer = create_bitmap(XWIN,YWIN);
	while(!finished){
		pthread_mutex_lock(&mutex);

		clear_to_color(buffer, BGC);
		line(buffer, 0, YWIN - LINE, XWIN, YWIN - LINE, 14); 

		// Scrivo il pianeta sul buffer.
		gui_planet(buffer);

		// Scrivo telescopi sul buffer.
		gui_telescopes(buffer);

		/* Interfaccia di modifica dei parametri */
		gui_param_interface(buffer);

		if(tel.elaborated == 1)
			textout_centre_ex(buffer, font, "ESC to exit the program",
						 XWIN / 2, BORDER, 14, 0);

		// mostro il mouse
		show_mouse(buffer);

		blit(buffer, screen, 0,0,0,0,XWIN,YWIN);
		
		pthread_mutex_unlock(&mutex);
		ptask_wait_for_period();
	}

	ptask_wait_for_period();

	fprintf(stderr, "gui has finished\n");
}
