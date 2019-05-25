#include "pmutex.h"
#include "ptask.h"
#include "tstat.h"
#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XWIN 1280
#define YWIN 720
#define OBS_SHAPE 300   /* Dimensione di un lato del quadrato si osservazione   */
//#define PI 3.1415
#define BGC 0    /* background color		*/
#define G 9.8    /* acceleration of gravity	*/
#define BASE 200  /* position of the floor	*/
#define BORDER 20    /* bordo tra le interfacce */
#define TOP 700  /* initial height of the ball	*/
#define XMIN 40  /* min position X of the ball	*/
#define XMAX 1200 /* max position Y of the ball	*/
#define PER 10   /* task period in ms		*/
#define DREL 10  /* realtive deadline in ms	*/
#define PRIO 80  /* task priority		*/

#define N 6


pthread_mutex_t mutex;
pthread_mutex_t acquisition[N];
pthread_mutex_t compute;
pthread_cond_t s;

int x_obs[N];   /* Valore sull'asse x dello spigolo in altro a destra della finestra di osservazione    */
int y_obs[N];   /* Valore sull'asse y dello spigolo in altro a destra della finestra di osservazione    */
int planet_x, planet_y; /* Coordinate del centroide del pianeta */

int centered[N]; /* Un valore per ogni telescopio, saranno uguali a zero il telescopio non ha inquadrato il pianeta,
                    uno altrimenti  */
int completed; /* Conta i telescopi che hanno completato l'osservazione  */
int noise_level;
int elaborated; /* A 1 se l'elaborazione finale è stata completata  */

BITMAP *planet_img; /* Bitmap di un pianeta */
BITMAP *sky; /* Bitmap con solo il pianeta, niente intefaccia   */
BITMAP *observation[N]; /* Bitmap delle osservazioni dei vari telescopi */
BITMAP *result; /* Bitmap con il risultato dell'elaborazione    */

void init_allegro(){
    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
    clear_to_color(screen, BGC);
    install_keyboard();
    install_mouse();
    enable_hardware_cursor();

    srand(time(NULL));

    textout_centre_ex(screen, font, "SPACE begin the observation", XWIN / 2, YWIN / 2,
                      14, 0);
    textout_centre_ex(screen, font, "ESC exit", XWIN / 2, YWIN / 2 + 30, 14, 0);
    line(screen, 0, YWIN - BASE, XWIN, YWIN - BASE, 14);
}

void init_semaphores(){
    int i;

    pmux_create_pi(&mutex);

    for(i=0; i<N; i++){
        //pmux_create_pi(&acquisition[i]);
        pthread_mutex_init(&acquisition[i], NULL);
        pthread_mutex_lock(&acquisition[i]);
    }

    pthread_mutex_init(&compute, NULL);
    pthread_mutex_lock(&compute);
}

void init_coordinates(){
    int i;

    for(i=0; i<N; i++){
        x_obs[i] = rand() % XWIN;
        if (x_obs[i] > (XWIN - OBS_SHAPE))
            x_obs[i] -= OBS_SHAPE;
        x_obs[i] = rand() % YWIN;
        if (y_obs[i] > YWIN - BASE)
            y_obs[i] = YWIN - BASE - OBS_SHAPE;
    }

    planet_x = 100;
    planet_y = 100 + OBS_SHAPE/2;
}

void init_bitmaps(){
    int i;

    planet_img = load_bitmap("../media/Jupiter.bmp", NULL);

    if(planet_img == NULL){
        printf("file not found\n");
        exit(1);
    }

    sky = create_bitmap(XWIN,YWIN);

    for(i=0; i<N; i++)
        observation[i] = create_bitmap(OBS_SHAPE, OBS_SHAPE);

    result = create_bitmap(OBS_SHAPE, OBS_SHAPE);
}

void init_parameters(){
    int i;

    for(i=0; i<N; i++)
        centered[i] = 0;
    
    completed = 0;
    noise_level = 500;
    elaborated = 0;
}

void init(){

    init_allegro();

    init_semaphores();

    init_coordinates();

    init_bitmaps();

    init_parameters();

    ptask_init(SCHED_FIFO, GLOBAL, PRIO_CEILING);
}

void gui(){
    int i;
    int x, y;
    int x1, y1;
    int x2, y2;
    BITMAP *buffer;

    buffer = create_bitmap(XWIN,YWIN);
    while(1){
        pthread_mutex_lock(&mutex);

        clear_to_color(buffer, BGC);
        line(buffer, 0, YWIN - BASE, XWIN, YWIN - BASE, 14); 

        // Scrivo il pianeta sul buffer.
        x = planet_x - OBS_SHAPE/2;
        y = planet_y - OBS_SHAPE/2;
        stretch_sprite(buffer, planet_img, x, y, 300, 300);

        // Realizzo il cielo

        clear_to_color(sky, BGC);
        stretch_sprite(sky, planet_img, x, y, 300, 300);

        // Scrivo telescopi sul buffer.
        for(i = 0; i < N; i++){
            x1 = 1 + i * (BASE - BORDER);
            y1 = YWIN - BASE + BORDER;
            x2 = (BASE - BORDER) * (i + 1); 
            y2 = YWIN - 1;

            textout_centre_ex(buffer, font, "Telescope", (x1 + x2) / 2, YWIN - BASE + 2,
                      100, 0);
            rect(buffer, x1, y1, x2, y2, 100);

            if(centered[i] == 1)
                stretch_sprite(buffer, observation[i], x1, y1, x2 - x1, y2 - y1);


            if((mouse_x > x1) && (mouse_x < x2)){
                if((mouse_y > y1) && (mouse_y < y2)){
                    // Scrivo observation window sul buffer.
                    rect(buffer, x_obs[i], y_obs[i], x_obs[i] + OBS_SHAPE, y_obs[i] + OBS_SHAPE, 14);
                }
            }
        }

        if(elaborated == 1){
            x1 = 1 + N * (BASE - BORDER);
            y1 = YWIN - BASE + BORDER;
            x2 = (BASE - BORDER) * (N + 1); 
            y2 = YWIN - 1;
            textout_centre_ex(buffer, font, "Result:", (x1 + x2) / 2, YWIN - BASE + 2,
                      14, 0);
            stretch_sprite(buffer, result, x1, y1, x2 - x1, y2 - y1);
            rect(buffer, x1 + 1, y1, x2, y2, 14);
        }
        

        // mostro il mouse
        show_mouse(buffer);

        blit(buffer, screen, 0,0,0,0,XWIN,YWIN);

        pthread_mutex_unlock(&mutex);
        ptask_wait_for_period();
    }
}

void planet(){
    int x = 100;
    int y = 100;
    float i = 0;

    while( i<SCREEN_W ){
        i+=0.25;
        pthread_mutex_lock(&mutex);
        planet_x = i + OBS_SHAPE/2;
        pthread_mutex_unlock(&mutex);
        
        ptask_wait_for_period();
    }
}

void start_acquisition(int i){
    int x, y;
    int c;
    int j;
    
    pthread_mutex_lock(&acquisition[i]);
    pthread_mutex_lock(&mutex);
    //fprintf(stderr, "ok start!\n");
    for(x = 1; x < OBS_SHAPE - 1; x++)
        for(y = 1; y < OBS_SHAPE - 1; y++){
            c = getpixel(sky, x_obs[i] + x, y_obs[i] + y);
            //fprintf(stderr, "i: %d, x: %d, y: %d\n", i, x, y);
            
            putpixel(observation[i], x, y, c);
            
        }

    for(j = 0; j < noise_level; j++){
        x = rand() % OBS_SHAPE;
        y = rand() % OBS_SHAPE;
        c = rand() % 16;
        putpixel(observation[i], x, y, c);
    }

    completed++;

    pthread_mutex_unlock(&mutex);
}

void end_acquisition(){
    pthread_mutex_lock(&mutex);

    if(completed == N){
        fprintf(stderr, "Hanno tutti completato!\n");
        pthread_mutex_unlock(&compute);
    }

    pthread_mutex_unlock(&mutex);
}

void telescope(){
    int i;
    i = ptask_get_index() - N - 1;
    //fprintf(stderr, "i: %d\n", i);

    while(1){
        start_acquisition(i);

        end_acquisition();
    }

    ptask_wait_for_period();
}

void telescope_motor(){
    int i;
    i = ptask_get_index() - 1;
    int x1, y1;
    int x2, y2;
    while(1){
        pthread_mutex_lock(&mutex);

        x1 = x_obs[i];
        y1 = y_obs[i];

        if(x_obs[i] < (planet_x - OBS_SHAPE/2)){
            x_obs[i] += 1;
        }
        else if(x1 > (planet_x - OBS_SHAPE/2))
            x_obs[i] -= 1;

        if(y1 < (planet_y - OBS_SHAPE/2))
            y_obs[i] += 1;
        else if(y1 > (planet_y - OBS_SHAPE/2))
            y_obs[i] -= 1;  

        if(centered[i] == 0)
            if((x_obs[i] == planet_x - OBS_SHAPE/2) && (y_obs[i] == planet_y - OBS_SHAPE/2)){
                //fprintf(stderr, "ok!\n");
                centered[i] = 1;
                pthread_mutex_unlock(&acquisition[i]);
            }
        
        pthread_mutex_unlock(&mutex);

        ptask_wait_for_period();
    }
}

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

void compute_result(){
    int x, y;
    int i;
    int c;
    int col[N];
    pthread_mutex_lock(&compute);
    pthread_mutex_lock(&mutex);

    fprintf(stderr, "Elaborazione!\n");

    for(x = 0; x < OBS_SHAPE; x++)
        for(y = 0; y < OBS_SHAPE; y++){
            c = 0;
            for(i = 0; i < N; i++){
                col[i] = getpixel(observation[i], x, y);
                //fprintf(stderr, "c: %d\n", c);
            }
            //fprintf(stderr, "final c: %d\n", c);

            c = find_majority_col(col);

            putpixel(result, x, y, c);
        }
    PALETTE pal;
    get_palette(pal);
    save_bitmap("result.bmp", result, pal);

    elaborated = 1;

    pthread_mutex_unlock(&mutex);
}

int main(void){
    int c;
    int i, k;
    int last_proc = 0;  /* last assigned processor      */
    int max_proc = ptask_getnumcores(); /* max number of procs  */

    init();
    do{
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }
    }while(k != KEY_SPACE);


    clear_to_color(screen, BGC);
    
    line(screen, 0, YWIN - BASE, XWIN, YWIN - BASE, 14); 
    tpars params = TASK_SPEC_DFL;
    params.period = tspec_from(PER, MILLI);
    params.rdline = tspec_from(DREL, MILLI);
    params.priority = PRIO;
    params.measure_flag = 1;
    params.act_flag = NOW;
    params.processor = last_proc++;
    if (last_proc >= max_proc)
        last_proc = 0;
    //ptask_param_init(params);
    i = ptask_create_param(planet, &params);
    if (i != -1) {
        printf("Planet, %d created and activated\n", i);
        
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    
    for(k = 0; k < N; k++){
        params.processor = last_proc++;
        if (last_proc >= max_proc)
            last_proc = 0;
        i = ptask_create_param(telescope_motor, &params);
        if (i != -1) {
            printf("Telescope motor, %d created and activated\n", i);
            
        } else {
            allegro_exit();
            printf("Error in creating task!\n");
            exit(-1);
        }     
    }

    for(k = 0; k < N; k++){
        params.processor = last_proc++;
        if (last_proc >= max_proc)
            last_proc = 0;
        i = ptask_create_param(telescope, &params);
        if (i != -1) {
            printf("Telescope acquisition, %d created and activated\n", i);
            
        } else {
            allegro_exit();
            printf("Error in creating task!\n");
            exit(-1);
        }     
    }

    params.processor = last_proc++;
    if (last_proc >= max_proc)
        last_proc = 0;
    //ptask_param_init(params);
    i = ptask_create_param(gui, &params);
    if (i != -1) {
        printf("gui, %d created and activated\n", i);
        
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    params.processor = last_proc++;
    if (last_proc >= max_proc)
        last_proc = 0;
    //ptask_param_init(params);
    i = ptask_create_param(compute_result, &params);
    if (i != -1) {
        printf("compute, %d created and activated\n", i);
        
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }


    
    readkey();
}