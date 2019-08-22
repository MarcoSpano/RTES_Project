#include "../lib/observatory.h"

//_____________________________________________________________________________
//_____________________________________________________________________________

//				INIT FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________

/**
 *  Intitializes all Allegro parameters. 
*/
void init_allegro(){
	allegro_init();
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
	clear_to_color(screen, BGC);
	install_keyboard();
	install_mouse();
	enable_hardware_cursor();

	srand(time(NULL));
}

/**
 * Intitializes all semphores. 
 * @param   : struct telescopes* t;	The telescopes.
*/
void init_semaphores(struct telescopes* t){
	int	i;	// A counter
	pmux_create_pi(&mutex);

	for(i = 0; i < N; i++){
		pmux_create_pi(&t->acquisition[i]);
		pmux_create_pi(&t->tracking[i]);
	}

	pthread_mutex_init(&t->compute, NULL);
	pthread_mutex_lock(&t->compute);
}

/**
 * Intitializes all coordinates (x,y) of the objects in the simulation. 
 * @param	: struct telescopes* t;	The telescopes.
*/
void init_coordinates(struct telescopes* t){
	int i;		// A counter
	int x, y;	// Observation window centroid coordinates

	for(i = 0; i < N; i++){

		x = rand() % (XDIAG - OBS_SHAPE/2) ;
		if (x <= OBS_SHAPE / 2)
			x += OBS_SHAPE / 2;

		t->x_obs[i] = x;

		y = rand() % (OBS_SHAPE / 2) - OBS_SHAPE / 4;
		y += 260;
	
		t->y_obs[i] = y;    
		t->x_angle[i] = 0;

		t->x_pred[i] = t->x_obs[i];
		t->y_pred[i] = t->y_obs[i];

		fprintf(stderr, "%d) x: %d, y: %d\n", i, t->x_pred[i], t->y_pred[i]);
	}

	planet_x = HUNDRED;
	planet_y = HUNDRED + OBS_SHAPE/2;
}

/** 
 * Intitializes planet velocity.
 * @param	: struct telescopes* t;	The telescopes.
*/
void init_velocity(struct telescopes* t){
	planet_vx = 0.25 / PER;
	planet_vy = 0;
}

/** 
 * Intitializes all Allegro bitmaps.
 * @param	: struct telescopes* t;	The telescopes.
*/
void init_bitmaps(struct telescopes* t){
	int     i;	// A counter

	planet_img = load_bitmap("../media/Jupiter.bmp", NULL);

	if(planet_img == NULL){
		printf("file not found\n");
		exit(1);
	}

	sky = create_bitmap(XWIN,YWIN);

	for(i=0; i<N; i++)
		t->observation[i] = create_bitmap(OBS_SHAPE, OBS_SHAPE);

	result = create_bitmap(OBS_SHAPE, OBS_SHAPE);
}

/**
 * Checks the value of the noise parameter n, if it isn't in the correct range,
 * a default value is used.
 * @param	: int n;	Value of noise. 
 * @return	: int;		The checked value.
*/
int check_value_noise(int n){
	if(n > MAX_NOISE)
		n = MAX_NOISE;
	else if(n < 0)
		n = DEFAULT_NOISE;
	
	return n;
}

/**
 * This functions checks the value of the motor parameter n, 
 * if it isn't in the correct range, a default value is used 
 * @param	: int n;	Value of motor parameter.
 * @return  : int n;	The checked value.
 */
int check_value_motor(int n){
	if(n > HUNDRED)
		n = HUNDRED;
	else if(n <= 0)
		n = DEFAULT_MOTOR;
	
	return n;
}

/** 
 * This function inizializes all telescope's parameters.
 * @param	: struct telescopes* t;	The telescopes.
*/
void init_parameters(struct telescopes* t){
	int     i;			// A counter
	char    *endptr;	// Needed for strtol
	int     n, m;		// Noise and Motor values

	for(i = 0; i < N; i++){
		t->telescope_state[i] = OBSERVATION;
		
		if(noise_modification[i][0] == 0)
			n = DEFAULT_NOISE;
		else{
			n = strtol(noise_modification[i], &endptr, 10);
			n = check_value_noise(n);
		}

		if(motor_modification[i][0] == 0)
			m = DEFAULT_MOTOR;
		else{
			m = strtol(motor_modification[i], &endptr, 10);
			m = check_value_motor(m);
		}
		t->noise_level[i] = n * NOISE_VAL_MULTIPLIER;
		t->motor_level[i] = m / 10;
		t->x_tel[i] = i * (BASE - BORDER) + (BASE - BORDER + 1) / 2;
		t->y_tel[i] = YWIN - LINE;
	}
	t->completed = 0;
	t->elaborated = 0;
	kp = kd = 1;

}

/**
 * Draws on the screen useful information to compile the dialog.
 */
void dial_info(){
	textout_centre_ex(screen, font,
		"You can selecet the value (in percentage) of telescopes parameters",
		XWIN / 2, BORDER, 14, 0);
	textout_centre_ex(screen, font,
		"[If you leave blank, default parameters will be used]", 
		XWIN / 2, 2*BORDER, 14, 0);
}

/**
 * Delivers a text dialog.
 * @param	: int i; 	Index for the row (y axis) of the text.
 * @param	: int x; 	X axis value of the text.
 * @param	: char s[]; String to draw on the screen.
 * @return	: DIALOG; 	The text dialog.
 */
DIALOG make_text(int i, int x, char s[]){
	return (DIALOG){d_text_proc, x, LINE + (i + 1) * BORDER, TEXT_W, TEXT_H,
			RED, BGC, 0, 0, 0, 0, s, NULL, NULL};
}

DIALOG make_edit(int i, int x, char s[]){
	return (DIALOG){d_edit_proc, x,
			LINE + (i + 1) * BORDER, TEXT_W, TEXT_H, 0, RED, 0, 0, 9, 0,
			s, NULL, NULL};
}

/**
 * Initializes the start dialog for parameters selection.
*/
void init_dial(){
	int     i;				// A counter
	int     j;				// A counter
	char    nl[6][14];		// Noise strings
	char    ml[6][14];		// Motor strings
	DIALOG  options[27];	// Start dialog

	options[0] = (DIALOG){d_box_proc, LINE, LINE, DIALOG_W, DIALOG_H, RED, BGC,
		0, 0, 0, 0, NULL, NULL, NULL};
	
	j = 1;
	for(i = 0; i < N; i++){
		sprintf(nl[i], "Noise level %d", i + 1);
		options[j] = make_text(i, NOISE_X, nl[i]);
		j++;
		options[j] = make_edit(i, NOISE_X + 2 * BORDER + TEXT_W,
			noise_modification[i]);
		j++;

		sprintf(ml[i], "Motor level %d", i + 1);
		options[j] = make_text(i, MOTOR_X, ml[i]);
		j++;
		options[j] = make_edit(i, MOTOR_X + N * BORDER, motor_modification[i]);
		j++;
	}
	options[j] = (DIALOG){d_button_proc, NOISE_X, LINE + DIALOG_H - BORDER * 2,
		BUTTON_W, BUTTON_H, GREEN, 0, 0, D_EXIT, 0, 0, "OK", NULL, NULL};
	j++;
	options[j] = (DIALOG){NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL};

	dial_info();

	do_dialog(options, -1);
}

/**
 * This function initializes all variables that will be used 
 */
void init(){

	init_allegro();

	init_dial();

	init_semaphores(&tel);

	init_coordinates(&tel);

	init_velocity(&tel);

	init_bitmaps(&tel);

	init_parameters(&tel);

	ptask_init(SCHED_FIFO, GLOBAL, PRIO_CEILING);
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//				MAIN FUNCTION COMPONENTS

//_____________________________________________________________________________
//_____________________________________________________________________________

/**
 * Checks the return value of ptask_create_prio() function. In case of an error
 * prints a message and exits.
 * @param	: int i;	Return value of ptask_create_prio() function.
 * @param	: char s[];	String with the name of the task to check.
 */
void check_task_creation(int i, char s[]){
	if (i != -1) {
		fprintf(stderr, "%s, %d created and activated\n", s, i);
	} else {
		allegro_exit();
		fprintf(stderr, "Error in creating task!\n");
		exit(-1);
	}
}

/**
 * A cycle that checks when a key is pressed, until ESC button is selected.
 */
void wait_for_esc(){
	int c;
	int k;

	do {
		k = 0;
		if (keypressed()) {
			c = readkey();
			k = c >> BIT_IN_BYTE;
		}
	
	} while (k != KEY_ESC);
}

/**
 * Creates tasks.
 */
void create_tasks(){
	int i;	// Return value of task creation
	int k;	// A counter

	/* Creates planet */
	i = ptask_create_prio(planet, PER, PRIO, NOW);
	check_task_creation(i, "Planet");
	
	/* Creates telescope motor */
	for(k = 0; k < N; k++){
		i = ptask_create_prio(telescope_motor, PER, PRIO, NOW);
		check_task_creation(i, "Telescope motor");     
	}
	
	/* Crea telescope */
	for(k = 0; k < N; k++){
		i = ptask_create_prio(telescope, PER, PRIO, NOW);
		check_task_creation(i, "Telescope acquisition");     
	}	

	/* Creates gui */
	i = ptask_create_prio(gui, PER, PRIO, NOW);
	check_task_creation(i, "gui");

	/* Creates elaboratori */
	i = ptask_create_prio(compute_result, PER, PRIO, NOW);
	check_task_creation(i, "compute");
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//				PLANET MOTION AND DETECTION FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________

/** 
 * Updates simulated planet position each period.
*/
void planet(){
	float   i;	// Planet's left border x position

	i = 0;

	while(planet_x < (XDIAG + OBS_SHAPE/2) && !finished){
		pthread_mutex_lock(&mutex);
		i += planet_vx * PER;
		planet_x = (int)(i + OBS_SHAPE/2);
		pthread_mutex_unlock(&mutex);
		
		ptask_wait_for_period();
	}
	fprintf(stderr, "planet has finished\n");
}

/**
 * Predicts the centroid of the image observed by telescope "i".
 * @param	: int i;	Index of the telescope.
 */
void centroid_prediction(int i){
	int     x, y;   		// Counters
	int     c;				// Color of a pixel
	int     x_tmp, y_tmp;	// Used to compute predicted centroid coordinates
	int     sum;			// Number of pixel read from sky
	int     lb, ub; 		/* Used to compute upper left corner coordinates of
								telescope's observation window */

	x_tmp = 0;
	y_tmp = 0;
	sum = 0;
	lb = tel.x_obs[i] - OBS_SHAPE / 2;
	ub = tel.y_obs[i] - OBS_SHAPE / 2;

	for(x = 1; x < OBS_SHAPE - 1; x++)
		for(y = 1; y < OBS_SHAPE - 1; y++){
			c = getpixel(sky, lb + x, ub + y);
			
			if(tel.telescope_state[i] != ACQUIRED)
				putpixel(tel.observation[i], x, y, c);

			if(c != BGC){
				x_tmp += x;
				y_tmp += y;
				sum++;

				if(tel.telescope_state[i] == OBSERVATION)
					tel.telescope_state[i] = TRACKING;
			}
		}

	if(tel.telescope_state[i] != OBSERVATION){
		x_tmp = x_tmp / sum;
		y_tmp = y_tmp / sum;

		tel.x_pred[i] = tel.x_obs[i] - OBS_SHAPE/2 + x_tmp;
		tel.y_pred[i] = tel.y_obs[i] - OBS_SHAPE/2 + y_tmp;
	}
}

/**
 * Defines what telescope "i" see.
 * @param   : int i;	Index of the telescope.
 */
void start_acquisition(int i){
	int x, y;	// Random coordinates
	int c;		// Random noise
	int j;		// A counter
	
	pthread_mutex_lock(&tel.acquisition[i]);
	pthread_mutex_lock(&mutex);
	
	centroid_prediction(i);

	// Adds noise to the observation, according to noise level parameter
	if(tel.telescope_state[i] != ACQUIRED)
		for(j = 0; j < tel.noise_level[i]; j++){
			x = rand() % OBS_SHAPE;
			y = rand() % OBS_SHAPE;
			c = rand() % HUNDRED/2 - HUNDRED;
			
			c += getpixel(tel.observation[i], x, y);
			putpixel(tel.observation[i], x, y, c);
		}

	pthread_mutex_unlock(&mutex);
}

void end_acquisition(int i){
	pthread_mutex_lock(&mutex);

	if(tel.telescope_state[i] != ACQUIRED)
		pthread_mutex_unlock(&tel.acquisition[i]);
	
	pthread_mutex_unlock(&mutex);
}

/**
 * Telescope logic task. 
 * It acquires images of the telescope's observation window.
 */
void telescope(){
	int i;  // Index of the task.

	i = ptask_get_index() - N - 1;

	while(!finished && (tel.telescope_state[i] != ACQUIRED)){
		start_acquisition(i);

		end_acquisition(i);

		ptask_wait_for_period();
	}

	if(tel.completed == N){
		fprintf(stderr, "Hanno tutti completato!\n");
		pthread_mutex_unlock(&tel.compute);
		tel.completed = 0;
	}

	ptask_wait_for_period();

	fprintf(stderr, "telescope %d has finished\n", i);
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//				TELESCOPES MOTOR LOGIC FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________

/**
 * Computes, for the x axis, the angle between observation's centroid 
 * and predicted planet's centroid.
 * @param	: struct telescopes* t;	The telescopes.
 * @param	: int i;				Telescope index.
 * @return	: double;				The angle.	
 */
double compute_xangle(struct telescopes* t, int i){
	double  m1;			// Slope of line between ground and observation centroid
	double	m2;			// Slope of line between ground and predicted centroid
	double	tn;			// Tangent of the angle

	int     y_obs, y_pred;
	int     delta_obs, delta_pred;

	delta_obs = t->x_obs[i] - t->x_tel[i];
	delta_pred = t->x_pred[i] - t->x_tel[i];
	
	y_obs = sqrt(RX * RX - delta_obs * delta_obs);
	y_pred = sqrt(RX * RX - delta_pred * delta_pred);

	if(delta_obs != 0)
		m1 = (double)(y_obs) / delta_obs;
	else
		m1 = HUNDRED;

	if(delta_pred != 0)
		m2 = (double)(y_pred) / delta_pred;
	else
		m2 = HUNDRED;
	
	tn = (m1 - m2) / (1 + m1 * m2);

	return atan(tn);
}

/**
 * Computes, for the y axis, the angle between observation's centroid 
 * and predicted planet's centroid.
 * @param	: struct telescopes* t;	The telescopes.
 * @param	: int i;				Telescope index.
 * @return	: double;				The angle.	
 */
double compute_yangle(struct telescopes* t, int i){
	double  m1;			// Slope of line between ground and observation centroid
	double	m2;			// Slope of line between ground and predicted centroid
	double	tn;			// Tangent of the angle

	int     x_obs, x_pred;
	int     delta_obs, delta_pred;

	delta_obs = t->y_obs[i] - t->y_tel[i];
	delta_pred = t->y_pred[i] - t->y_tel[i];

	x_obs = sqrt(RY * RY - delta_obs * delta_obs);
	x_pred = sqrt(RY * RY - delta_pred * delta_pred);
	
	m1 = (double)(delta_obs) / (x_obs);
	m2 = (double)(delta_pred) / (x_pred);
	
	tn = (m1 - m2) / (1 + m1 * m2);

	return atan(tn);
}

/**
 * Motor that moves the observation window on the x axis by a certain angle.
 * @param	: double angle;	Angle.
 * @param	: int i;		Index of the telescope.
 */
void xmotor(double angle, int i){
	float   x;		// Angle in radiants
	float   delta;	// Movement value in pixels

	x = (angle * RX) / 180;

	delta = tel.motor_level[i] * x;
	if(delta > 0 && delta < 1)
		delta = 1;
	else if(delta < 0 && delta > -1)
		delta = -1;

	tel.x_obs[i] += delta;
}

/**
 * Motor that moves the observation window on the y axis by a certain angle.
 * @param	: double angle;	Angle.
 * @param	: int i;		Index of the telescope.
 */
void ymotor(double angle, int i){
	float y;		// Angle in radiants
	float delta;	// Movement value in pixels

	y = (RY * angle) / 180;

	delta = tel.motor_level[i] * y;

	if(delta > 0 && delta < 1)
		delta = 1;
	else if(delta < 0 && delta > -1)
		delta = -1;

	tel.y_obs[i] -= delta;
}

/**
 * Telescope's motor task.
 * Updates telescope's observetion window.
 */
void telescope_motor(){
	int     i;		// Task index
	int     x, y;   // Current coordinates
	double  angle;	// Angle to give to motors

	i = ptask_get_index() - 1;

	while(tel.telescope_state[i] != ACQUIRED && !finished){
		pthread_mutex_lock(&tel.tracking[i]);
		pthread_mutex_lock(&mutex);
		
		x = tel.x_obs[i];
		y = tel.y_obs[i];

		if(abs(tel.x_pred[i] - tel.x_obs[i]) > 0){
			angle = compute_xangle(&tel, i);
			xmotor(angle, i);
		}
		if(abs(tel.y_pred[i] - tel.y_obs[i]) > 0){
			angle = compute_yangle(&tel, i);
			ymotor(angle, i);
		}
		
		if(tel.telescope_state[i] == TRACKING){
			if(tel.x_obs[i] == tel.x_pred[i]){
				if(tel.y_obs[i] == tel.y_pred[i]){
					tel.telescope_state[i] = ACQUIRED;
					tel.completed++;
				}
			}
		}

		if(tel.telescope_state[i] != ACQUIRED)
			pthread_mutex_unlock(&tel.tracking[i]);

		pthread_mutex_unlock(&mutex);
		
		ptask_wait_for_period();
	}

	fprintf(stderr, "motor %d has finished\n", i);
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//				IMAGE ELABORATION FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________

/**
 * Given a color vector, finds the most recurrent value.
 * @param	: int col[];	Vector of colors.
 * @return	: int;			Most frequent color.
 */
int find_majority_col(int col[]){
	int i, j;	// Counters
	int n;		// Counter
	int c;		// Most frequent color
	int max;	// Frequency of c 

	c = 0;
	max = 0;
	for(i = 0; i < N; i++){
		n = 0;
		for(j = i; j < N; j++){
			if(col[i] == col[j])
				n++;
		}
		if(n > max){
			max = n;
			c = col[i];
		}
	}

	return c;
}

/**
 * Computes the result image of the planet observed by all telescopes.
 */
void compute_result(){
	int     x, y;	// Counters
	int     i;		// A counter
	int     c;		// Most frequent color of a pixel in all observations
	int     col[N];	// Colors of a pixel in all observations
	PALETTE pal;	// Color palette

	pthread_mutex_lock(&tel.compute);
	pthread_mutex_lock(&mutex);

	fprintf(stderr, "Elaborazione...\n");

	for(x = 0; x < OBS_SHAPE; x++)
		for(y = 0; y < OBS_SHAPE; y++){
			c = 0;
			for(i = 0; i < N; i++){
				col[i] = getpixel(tel.observation[i], x, y);
			}

			c = find_majority_col(col);
			putpixel(result, x, y, c);
		}
	
	get_palette(pal);
	save_bitmap("../media/result.bmp", result, pal);
	tel.elaborated = 1;

	pthread_mutex_unlock(&mutex);

	fprintf(stderr, "compute has finished\n");
}