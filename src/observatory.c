#include "../lib/observatory.h"

//_____________________________________________________________________________
//_____________________________________________________________________________

//				INIT FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________


void init_allegro(){
    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    install_mouse();
    enable_hardware_cursor();

    srand(time(NULL));
}

void init_semaphores(struct telescopes* t){
    int i;

    pmux_create_pi(&mutex);

    for(i=0; i<N; i++){
        pmux_create_pi(&t->acquisition[i]);
        pmux_create_pi(&t->tracking[i]);
    }

    pthread_mutex_init(&t->compute, NULL);
    pthread_mutex_lock(&t->compute);
}

void init_coordinates(struct telescopes* t){
    int i;

    for(i=0; i<N; i++){
        t->x_obs[i] = rand() % XWIN + OBS_SHAPE;
        if (t->x_obs[i] > XDIAG)
            t->x_obs[i] -= OBS_SHAPE*2;

        t->y_obs[i] = rand() % YWIN + OBS_SHAPE/2;
        if (t->y_obs[i] > YWIN - BASE - OBS_SHAPE/2)
            t->y_obs[i] = YWIN - BASE - OBS_SHAPE;
            
        t->x_angle[i] = 0;

        t->x_pred[i] = t->x_obs[i];
        t->y_pred[i] = t->y_obs[i];
    }

    planet_x = 100;
    planet_y = 100 + OBS_SHAPE/2;
}

void init_velocity(struct telescopes* t){
    int i;

    planet_vx = 0.25 / PER;
    planet_vy = 0;
}

void init_bitmaps(struct telescopes* t){
    int i;

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

int check_value_noise(int n){
    if(n > MAX_NOISE)
        n = MAX_NOISE;
    else if(n < 0)
        n = DEFAULT_NOISE;
    
    return n;
}

int check_value_motor(int n){
    if(n > MAX_NOISE)
        n = MAX_NOISE;
    else if(n <= 0)
        n = DEFAULT_MOTOR;
    
    return n;
}

void init_parameters(struct telescopes* t){
    int i;
    char *endptr;
    int n, m;

    for(i=0; i<N; i++){
        t->telescope_state[i] = OBSERVATION;
        //centered[i] = 0;
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
        t->x_tel[i] = i * (BASE - BORDER) + (BASE - BORDER + 1)/2;
        t->y_tel[i] = YWIN - LINE;
    }
    t->completed = 0;
    t->elaborated = 0;
    kp = kd = 1;

}

void init_dial(){
    int i;
    int j;
    char nl[6][14];
    char ml[6][14];

    DIALOG options[27];
    options[0] = (DIALOG){d_box_proc, LINE, LINE, DIALOG_W, DIALOG_H, 15, BGC,
     0, 0, 0, 0, NULL, NULL, NULL};
    j = 1;
    for(i = 0; i < N; i++){
        sprintf(nl[i], "Noise level %d", i + 1);
        options[j] = (DIALOG){d_text_proc, LINE + BORDER, LINE + (i + 1)*BORDER,
         TEXT_W, TEXT_H, 15, BGC, 0, 0, 0, 0, nl[i], NULL, NULL};
        j++;
        options[j] = (DIALOG){d_edit_proc, LINE + 3*BORDER + TEXT_W,
         LINE + (i + 1)*BORDER, TEXT_W, TEXT_H, 0, 15, 0, 0, 9, 0,
          noise_modification[i], NULL, NULL};
        j++;
        sprintf(ml[i], "Motor level %d", i + 1);
        options[j] = (DIALOG){d_text_proc, LINE + N*TEXT_W, LINE + (i + 1)*BORDER,
         TEXT_W, TEXT_H, 15, BGC, 0, 0, 0, 0, ml[i], NULL, NULL};
        j++;
        options[j] = (DIALOG){d_edit_proc, LINE + N*BORDER + N*TEXT_W,
         LINE + (i + 1)*BORDER, TEXT_W, TEXT_H, 0, 15, 0, 0, 9, 0,
          motor_modification[i], NULL, NULL};
        j++;
    }
    options[j] = (DIALOG) {d_button_proc, LINE + BORDER, LINE + DIALOG_H - BORDER*2,
     40, 25, 10, 0, 0, D_EXIT, 0, 0, "OK", NULL, NULL};
    j++;
    options[j] = (DIALOG) {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL};

    textout_centre_ex(screen, font,
     "You can selecet the value (in percentage) of telescopes parameters",
      XWIN / 2, BORDER, 14, 0);
    textout_centre_ex(screen, font,
     "[If you leave blank, default parameters will be used]", 
     XWIN / 2, 2*BORDER, 14, 0);
    do_dialog(options, -1);
}

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

//				PLANET MOTION AND DETECTION FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________


void planet(){
    int x = 100;
    int y = 100;
    float i = 0;

    while(planet_x < XDIAG + OBS_SHAPE/2){
        pthread_mutex_lock(&mutex);
        i += planet_vx * PER;
        planet_x = i + OBS_SHAPE/2;
        pthread_mutex_unlock(&mutex);
        
        ptask_wait_for_period();
    }
    fprintf(stderr, "planet has finished\n");
}

void start_acquisition(int i){
    int x, y;
    int c;
    int j;
    int x_tmp, y_tmp;
    int sum;
    
    pthread_mutex_lock(&tel.acquisition[i]);
    pthread_mutex_lock(&mutex);
    //fprintf(stderr, "ok start!\n");

    x_tmp = 0;
    y_tmp = 0;
    sum = 0;

    for(x = 1; x < OBS_SHAPE - 1; x++)
        for(y = 1; y < OBS_SHAPE - 1; y++){
            c = getpixel(sky, tel.x_obs[i] - OBS_SHAPE/2 + x, tel.y_obs[i] - OBS_SHAPE/2 + y);
            //fprintf(stderr, "i: %d, x: %d, y: %d\n", i, x, y);
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

    if(tel.telescope_state[i] != ACQUIRED)
        for(j = 0; j < tel.noise_level[i]; j++){
            x = rand() % OBS_SHAPE;
            y = rand() % OBS_SHAPE;
            c = rand() % 50 - 100;
            //if(c == 1)
                c += getpixel(tel.observation[i], x, y);
            putpixel(tel.observation[i], x, y, c);
        }

    pthread_mutex_unlock(&mutex);
}

void end_acquisition(int i){
    pthread_mutex_lock(&mutex);

    if(tel.completed == N){
        fprintf(stderr, "Hanno tutti completato!\n");
        pthread_mutex_unlock(&tel.compute);
        tel.completed = 0;
    }
    if(tel.telescope_state[i] != ACQUIRED)
        pthread_mutex_unlock(&tel.acquisition[i]);
    pthread_mutex_unlock(&mutex);
}

void telescope(){
    int i;
    i = ptask_get_index() - N - 1;

    while(1){
        start_acquisition(i);

        end_acquisition(i);

        ptask_wait_for_period();
    }

    ptask_wait_for_period();

    fprintf(stderr, "telescope %d has finished\n", i);
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//				TELESCOPES MOTOR LOGIC FUNCTIONS

//_____________________________________________________________________________
//_____________________________________________________________________________


double square_difference(int x1, int y1, int x2, int y2){
    int sqdiff_x, sqdiff_y;
    sqdiff_x = (x2 - x1) * (x2 - x1);
    sqdiff_y = (y2 - y1) * (y2 - y1);

    return sqrt(sqdiff_x + sqdiff_y);
}

double compute_xangle(struct telescopes* t, int i){
    
    double m1, m2, tn;
    int y_obs, y_pred;
    y_obs = sqrt(RX*RX - (t->x_obs[i]-t->x_tel[i])*(t->x_obs[i]-t->x_tel[i]));
    y_pred = sqrt(RX*RX - (t->x_pred[i]-t->x_tel[i])*(t->x_pred[i]-t->x_tel[i]));

    if((t->x_obs[i] - t->x_tel[i]) != 0)
        m1 = (double)(y_obs) / (t->x_obs[i] - t->x_tel[i]);
    else
        m1 = 100;

    if((t->x_pred[i]  - t->x_tel[i]) != 0)
        m2 = (double)(y_pred) / (t->x_pred[i]  - t->x_tel[i]);
    else
        m2 = 100;
    
    tn = (m1 - m2)/(1 + m1*m2);

    return atan(tn);
}

double compute_yangle(struct telescopes* t, int i){
    
    double m1, m2, tn;
    int x_obs, x_pred;
    x_obs = sqrt(RY*RY - (t->y_obs[i]-t->y_tel[i])*(t->y_obs[i]-t->y_tel[i]));
    x_pred = sqrt(RY*RY - (t->y_pred[i]-t->y_tel[i])*(t->y_pred[i]-t->y_tel[i]));
    
    m1 = (double)(t->y_obs[i] - t->y_tel[i]) / (x_obs);
    m2 = (double)(t->y_pred[i] - t->y_tel[i]) / (x_pred);

    //fprintf(stderr, "%d, m1: %lf + m2: %lf\n", i, m1, m2);
    
    tn = (m1 - m2)/(1 + m1*m2);

    return atan(tn);
}

void xmotor(double angle, int i){
    float x;
    float delta;

    x = (angle * RX)/180;

    //fprintf(stderr, "Aggiornamento: %lf\n", x);

    delta = tel.motor_level[i]*x;
    if(delta > 0 && delta < 1)
        delta = 1;
    else if(delta < 0 && delta > -1)
        delta = -1;

    tel.x_obs[i] += delta;
}

void ymotor(double angle, int i){
    float y;
    float delta;

    y = (RY * angle)/180;
    fprintf(stderr, "Aggiornamento: %lf\n", y);

    delta = tel.motor_level[i]*y;
    if(delta > 0 && delta < 1)
        delta = 1;
    else if(delta < 0 && delta > -1)
        delta = -1;

    tel.y_obs[i] -= delta;
}

void telescope_motor(){
    int i;
    i = ptask_get_index() - 1;
    int x, y;   // Posizione attuale
    double angle;

    while(tel.telescope_state[i] != ACQUIRED){
        pthread_mutex_lock(&tel.tracking[i]);
        pthread_mutex_lock(&mutex);
        
        x = tel.x_obs[i];
        y = tel.y_obs[i];

        if(abs(tel.x_pred[i] - tel.x_obs[i]) > 0){
            angle = compute_xangle(&tel, i);
            xmotor(angle, i);
            //fprintf(stderr, "%d, xangle: %lf\n", i, angle);
        }
        if(abs(tel.y_pred[i] - tel.y_obs[i]) > 0){
            angle = compute_yangle(&tel, i);
            ymotor(angle, i);
            //fprintf(stderr, "%d, yangle: %lf\n", i, angle/3.14*180);
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

int find_majority_col(int col[]){
    int i, j;
    int n;
    int c;
    int max;

    c = 0;
    max = 0;
    for(i = 0; i < N; i++){
        n = 0;
        for(j = i; j < N; j++){
            //fprintf(stderr, "i: %d, j: %d\n", col[i], col[j]);

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

int mean_pixel(int col[]){
    int i, j;
    int n;
    int c;
    int sum = 0;

    for(int i=0; i<N; i++){
        sum += col[i];
    }

    c = sum / N;

    return c;
}

void compute_result(){
    int x, y;
    int i;
    int c;
    int col[N];
    pthread_mutex_lock(&tel.compute);
    pthread_mutex_lock(&mutex);

    fprintf(stderr, "Elaborazione...\n");

    for(x = 0; x < OBS_SHAPE; x++)
        for(y = 0; y < OBS_SHAPE; y++){
            c = 0;
            for(i = 0; i < N; i++){
                col[i] = getpixel(tel.observation[i], x, y);
                //fprintf(stderr, "c: %d\n", c);
            }
            //fprintf(stderr, "final c: %d\n", c);

            c = find_majority_col(col);
            //c = mean_pixel(col);

            putpixel(result, x, y, c);
        }
    PALETTE pal;
    get_palette(pal);
    save_bitmap("../media/result.bmp", result, pal);

    tel.elaborated = 1;

    pthread_mutex_unlock(&mutex);

    fprintf(stderr, "compute has finished\n");
}