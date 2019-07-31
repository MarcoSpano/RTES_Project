#ifndef OBSERVATORY_H

#include "pmutex.h"
#include "ptask.h"
#include "tstat.h"
#include <allegro.h>
#include "../lib/gui.h"
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XWIN 1280
#define YWIN 720
#define OBS_SHAPE 300   /* Dimensione di un lato del quadrato si osservazione*/
#define BGC 0    /* background color		*/
#define G 9.8    /* acceleration of gravity	*/
#define BASE 200  /* position of the floor	*/
#define LINE 200
#define BORDER 20    /* bordo tra le interfacce */
#define PER 10   /* task period in ms		*/
#define DREL 10  /* realtive deadline in ms	*/
#define PRIO 80  /* task priority		*/
#define MAX_NOISE 100 /* Massimo rumore in percentuale */
#define DEFAULT_NOISE 50 /* Rumore standard in percentuale */
#define NOISE_VAL_MULTIPLIER 25 /* Usato per moltiplicare la percentale di rumore */

#define OBSERVATION 0
#define TRACKING 1
#define ACQUIRED 2
#define COMPLETED 3

#define XDIAG XWIN - BASE
#define YDIAG 1
#define WDIAG BASE
#define HDIAG YWIN - LINE - 1
#define TBLOCK BORDER * 4
#define RY 520
#define RX 1080

#define DIALOG_W XWIN - 2*LINE
#define DIALOG_H YWIN - 2*LINE
#define TEXT_W 80
#define TEXT_H 60

#define N 6



pthread_mutex_t mutex;

struct telescopes{
    pthread_mutex_t acquisition[N];
    pthread_mutex_t tracking[N];
    pthread_mutex_t compute;

    int x_obs[N];   /* Valore sull'asse x del centroide della finestra di osservazione    */
    int y_obs[N];   /* Valore sull'asse y del centroide della finestra di osservazione    */
    float x_tel[N];   /* Valore sull'asse x della posizione del telescopio    */
    float y_tel[N];   /* Valore sull'asse y della posizione del telescopio    */
    float x_angle[N]; /* Valore sull'asse x dell'angolo relativo al motore dei telescopi    */

    int telescope_state[N];  /* Stato di ogni telescopio */
    int x_pred[N];   /* Valore sull'asse x del centroide predetto del pianeta    */
    int y_pred[N];   /* Valore sull'asse y del centroide predetto del pianeta    */

    int completed; /* Conta i telescopi che hanno completato l'osservazione  */
    int noise_level[N]; /* Livello del rumore percepito da ogni telescopio  */
    int motor_level[N]; /* Motor speed level */
    int elaborated; /* A 1 se l'elaborazione finale è stata completata  */

    BITMAP *observation[N]; /* Bitmap delle osservazioni dei vari telescopi */
}tel;

int planet_x, planet_y; /* Coordinate del centroide del pianeta */
float planet_vx;   /* Velocità del pianeta sull'asse x    */
float planet_vy;   /* Velocità del pianeta sull'asse y    */

int kp, kd; /* Parametri di controllo del telescope motor   */

BITMAP *planet_img; /* Bitmap di un pianeta */
BITMAP *sky; /* Bitmap con solo il pianeta, niente intefaccia. E' ciò che un telescopio può vedere  */
BITMAP *result; /* Bitmap con il risultato dell'elaborazione    */

char noise_modification[6][3]; /* Contiene il valore per modificare il rumore */
char motor_modification[6][3]; /* Contiene il valore per modificare il la velocità del motore */

extern void init();

extern void planet();

extern void telescope();

extern void telescope_motor();

extern void compute_result();


#endif