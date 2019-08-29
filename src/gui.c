#include "../lib/observatory.h"

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				This file contains all functions related to graphical
//				appearence of the simulation.
//_____________________________________________________________________________
//_____________________________________________________________________________


/**
 * Handles graphic representation of the planet.
 * @param	: BITMAP *buffer;	A bitmap that will be drawn on screen.
 */
void gui_planet(BITMAP *buffer){
	int	x, y;	// Planet's image upper left corner coordinates

	if(planet_x < XDIAG + OBS_SHAPE / 2){
		x = planet_x - OBS_SHAPE / 2;
		y = planet_y - OBS_SHAPE / 2;
		stretch_sprite(buffer, planet_img, x, y, OBS_SHAPE, OBS_SHAPE);
	}

	clear_to_color(sky, BGC);
	stretch_sprite(sky, planet_img, x, y, OBS_SHAPE, OBS_SHAPE);
}

/**
 * Handles graphic representation of a telescope's observation window and
 * shows the line between its centroid and the position of the telescope.
 * @param	: BITMAP *buffer;	A bitmap that will be drawn on screen.
 * @param	: int i;			Index of the telecope.
 */
void telescope_on_sky(BITMAP *buffer, int i){
	rect(buffer, tel.x_obs[i] - OBS_SHAPE / 2, tel.y_obs[i] - OBS_SHAPE / 2,
		tel.x_obs[i] + OBS_SHAPE / 2, tel.y_obs[i] + OBS_SHAPE / 2, WHITE);
	line(buffer, tel.x_tel[i], tel.y_tel[i], tel.x_obs[i], tel.y_obs[i],
		YELLOW);
	line(buffer, tel.x_tel[i], tel.y_tel[i], tel.x_pred[i], tel.y_pred[i], 30);
}

/**
 * Handles graphic representation of the result image.
 * @param	: BITMAP *buffer;	A bitmap that will be drawn on screen.
 */
void show_result(BITMAP *buffer){
	int	x1, y1, x2, y2;	// Coordinates of the result image corners

	x1 = 1 + N * (BASE - BORDER);
	y1 = YWIN - LINE + BORDER;
	x2 = (BASE - BORDER) * (N + 1); 
	y2 = y1 + BASE - BORDER - 1;
	textout_centre_ex(buffer, font, "Result:", (x1 + x2) / 2, YWIN - LINE + 2,
		YELLOW, 0);
	stretch_sprite(buffer, result, x1, y1, x2 - x1, y2 - y1);
	rect(buffer, x1 + 1, y1, x2, y2, YELLOW);
}

/**
 * Handles graphic representation of the telescopes.
 * @param	: BITMAP *buffer;	A bitmap that will be drawn on screen.
 */
void gui_telescopes(BITMAP *buffer){
	int	i;				// A counter
	int	x1, y1, x2, y2;	// Coordinates of each telescope image coordinates

	for(i = 0; i < N; i++){
			x1 = 1 + i * (BASE - BORDER);
			y1 = YWIN - LINE + BORDER;
			x2 = (BASE - BORDER) * (i + 1); 
			y2 = y1 + BASE - BORDER - 1;

			textout_centre_ex(buffer, font, "Telescope", (x1 + x2) / 2,
								YWIN - LINE + 2, HUNDRED, 0);

			stretch_sprite(buffer, tel.observation[i], x1, y1, x2 - x1,
				y2 - y1);

			rect(buffer, tel.x_pred[i], tel.y_pred[i], tel.x_pred[i] + 2,
				tel.y_pred[i] + 2, 40);

			rect(buffer, x1, y1, x2, y2, HUNDRED);

			if((mouse_x > x1) && (mouse_x < x2)){
				if((mouse_y > y1) && (mouse_y < y2)){
					telescope_on_sky(buffer, i);
				}
			}
		}

		if(tel.elaborated == 1){
			show_result(buffer);
		}
}

/**
 * Handles graphic representation of noise and motor parameters.
 * @param	: BITMAP *buffer;	A bitmap that will be drawn on screen.
 */
void gui_param_interface(BITMAP *buffer){
	int		i;					// A counter
	char	s[T_NAME_LEN];		// Telescope name string
	char	nl[MNL_STRING_LEN];	// Noise level string
	char	ml[MNL_STRING_LEN];	// Motor level string

	rectfill(buffer, XDIAG, YDIAG, XWIN, YDIAG+HDIAG, BGC);
	rect(buffer, XDIAG, YDIAG, XWIN - 1, YDIAG + HDIAG, WHITE);

	for(i = 0; i < N; i++){
		sprintf(s, "Telescope %d", i+1);
		textout_ex(buffer, font, s, XDIAG + BORDER,
			YDIAG + TBLOCK * (i) + BORDER, WHITE, 0);

		sprintf(nl, "noise level: %d%%",
			tel.noise_level[i] / NOISE_VAL_MULTIPLIER);
		textout_ex(buffer, font, nl, XDIAG + BORDER * 2,
			YDIAG + TBLOCK * (i) + BORDER * 2, WHITE, 0);

		sprintf(nl, "motor level: %d%%", tel.motor_level[i] * BASE10);
		textout_ex(buffer, font, nl, XDIAG + BORDER * 2,
			YDIAG + TBLOCK * (i) + BORDER * (2 + 1), WHITE, 0);
	}
}

/**
 * Handles graphic representation of the instruction to exit the program.
 * @param	: BITMAP *buffer;	A bitmap that will be drawn on screen.
 */
void gui_end(BITMAP *buffer){
	if(tel.elaborated == 1)
		textout_centre_ex(buffer, font, "ESC to exit the program", XWIN / 2,
			BORDER, YELLOW, 0);
}

/**
 * Handles graphic representation of the application.
 */
void gui(){
	BITMAP	*buffer;	// The only bitmap that will be directly drawn on screen

	buffer = create_bitmap(XWIN,YWIN);
	while(!finished){
		pthread_mutex_lock(&mutex);

		clear_to_color(buffer, BGC);
		line(buffer, 0, YWIN - LINE, XWIN, YWIN - LINE, YELLOW); 

		gui_planet(buffer);

		gui_telescopes(buffer);

		gui_param_interface(buffer);

		gui_end(buffer);

		show_mouse(buffer);

		blit(buffer, screen, 0, 0, 0, 0, XWIN, YWIN);
		
		pthread_mutex_unlock(&mutex);
		
		ptask_wait_for_period();
	}

	fprintf(stderr, "gui has finished\n");
}
