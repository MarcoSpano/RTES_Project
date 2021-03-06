#ifndef OBSERVATORY_H

#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pmutex.h"
#include "ptask.h"
#include "tstat.h"
#include "../lib/gui.h"

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				This file contains all the definitions and variables
//				that will be used. It contains observatory.c functions
//				declarations.
//_____________________________________________________________________________
//_____________________________________________________________________________

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				INIT DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________


#define XWIN 1280		// Horizontal pixel number of the application window
#define YWIN 720		// Vertical pixel number of the application window
#define OBS_SHAPE 300	// Number of pixel of a side of the observation window
#define T_CENTR 40		// Number of pixel of a side of the predicted centroid
#define BGC 0    		// Background color
#define BASE 200		// Y axis position of the floor from the bottom
#define LINE 200		// Y axis position of separation line frome the bottom
#define BORDER 20    	// Border size in pixels
#define PER 10   		// Task period in ms
#define ACQ_PER 20  	// Aquisition task period in ms
#define PRIO 80  		// task priority
#define N 6				// Number of telescopes

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				PARAMETERS RELATED DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define MAX_NOISE 100 				// Max noise in precentage
#define DEFAULT_NOISE 50 			// Standard noise in percentage
#define NOISE_VAL_MULTIPLIER 100 	// Used to multiply noise percentage
#define DEFAULT_MOTOR 10 			// Standard motor value in percentage

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				USEFUL NUMBERS DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define HUNDRED 100
#define A_QUARTER 0.25
#define BASE10 10
#define FLAT_ANGLE 180
#define BIT_IN_BYTE 8	// Number of bits in a byte

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				ALLEGRO COLORS DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define WHITE 15		// White value in allegro's palette
#define YELLOW 14		// Yellow value in allegro's palette
#define GREEN 10		// Green value in allegro's palette

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				STATE DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define OBSERVATION 0	// Observarion state value
#define TRACKING 1		// Tracking state value
#define ACQUIRED 2		// Acquired state value
#define COMPLETED 3		// Completed state value

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				INFORMATION INTERFACE DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define XDIAG XWIN - BASE		// X axis position of information zone rect
#define YDIAG 1					// Y axis position of information zone rect
#define WDIAG XWIN - XDIAG		// Width of information zone
#define HDIAG YWIN - LINE - 1	// Height of information zone
#define TBLOCK BORDER * 4		// Shape of a block
#define MNL_STRING_LEN 17		// Length of noise and motor level strings
#define T_NAME_LEN 12			// Length of telescope name string

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				OBSERVABLE SPACE DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define RY 520				// Y axis (pixel) length of observable space
#define RX 1080				// X axis (pixel) length of observable space
#define PLANET_LIMIT 1180	// X axis value, after this the planet is no more drawn

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				DIALOG RELATED DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

#define DIALOG_W XWIN - 2 * LINE	// Dialog width
#define DIALOG_H YWIN - 2 * LINE	// Dialog height
#define TEXT_W 80					// Dialog text width
#define TEXT_H 10					// Dialog text height
#define NOISE_X LINE + BORDER		// Initial x axis position of dialog noise information
#define MOTOR_X LINE + N * TEXT_W	// Initial x axis position of dialog motor information
#define BUTTON_W 40					// Dialog button width
#define BUTTON_H 25					// Dialog button height
#define PARAM_STRING_LEN 14			// Length of the string to be written
#define DIGITS_IN_EDIT 3			// Number of digits in an edit dialog

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				STRUCTURES DEFINITIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

struct telescopes{
	pthread_mutex_t compute;		// compute() task semaphore

	int		x_obs[N];	// X axis value of observation windows centroids
	int		y_obs[N];	// Y axis value of observation windows centroids
	float	x_tel[N];	// X axis position of telescopes
	float	y_tel[N];	// Y axis position of telescopes
	int		x_pred[N];	// X axis position of predected planet's centroid
	int		y_pred[N];	// Y axis position of predected planet's centroid

	int		telescope_state[N];  // State of each telescope

	int		noise_level[N];	// Noise level of each telescope
	int		motor_level[N];	// Motor speed level of each telescope

	int		completed;	// Counts telescopes that have competed the observation
	int		elaborated; // Equals 1 if final elaboration has been completed

	BITMAP  *observation[N];	// Bitmaps of observetions
}tel;

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				VARIABLES
//_____________________________________________________________________________
//_____________________________________________________________________________

pthread_mutex_t mutex;

int		planet_x, planet_y;	// Planet's centroid coordinates
float	planet_vx;			// X axis planet velocity
float	planet_vy;			// Y axis planet velocity

BITMAP  *planet_img;	// Planet bitmap
BITMAP  *sky;			// Sky bitmap. It's what a telescope can see (contains only the planet)
BITMAP  *result;		// Result image bitmap

char    noise_modification[6][16];	// Contains values for noise level modification
char    motor_modification[6][16];	// Contains values for motor level modification

int finished;	// Equals 1 if the program must end (ESC is pressed)

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				FUCTIONS DECLARATIONS
//_____________________________________________________________________________
//_____________________________________________________________________________

extern void init();

extern void create_tasks();

extern void check_task_creation(int i, char s[]);

extern void wait_for_esc();

extern void planet();

extern void telescope();

extern void telescope_motor();

extern void compute_result();

#endif