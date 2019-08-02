#include "../lib/observatory.h"

int main(void){
    int c;
    int i, k;
    int last_proc = 0;  /* last assigned processor  */
    int max_proc = ptask_getnumcores(); /* max number of procs  */
    char s[12];
    char nl[15];
    int x1, y1, x2, y2;

    finished = 0;
    init();
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
            
            sprintf(s, "Telescope %d", i+1);
            textout_ex(screen, font, s, BORDER + i * XWIN/N, YWIN - LINE + BORDER, 15, 0);
            sprintf(nl, "noise level: %d", tel.noise_level[i]);
            textout_ex(screen, font, nl, BORDER * 2 + i * XWIN/N, YWIN -LINE + BORDER * 2, 15, 0);
            line(screen, (i+1) * XWIN/N, YWIN -LINE, (i+1) * XWIN/N, YWIN, 15);
        }
    }while(k != KEY_SPACE);
    
    clear_to_color(screen, BGC);
    
    line(screen, 0, YWIN - BASE, XWIN, YWIN - BASE, 14); 

    i = ptask_create_prio(planet, PER, PRIO, NOW);
    if (i != -1) {
        printf("Planet, %d created and activated\n", i);
        
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    /* Crea telescope motor */
    for(k = 0; k < N; k++){
        i = ptask_create_prio(telescope_motor, PER, PRIO, NOW);
        if (i != -1) {
            printf("Telescope motor, %d created and activated\n", i);
        } else {
            allegro_exit();
            printf("Error in creating task!\n");
            exit(-1);
        }     
    }
    /* Crea telescope */
    for(k = 0; k < N; k++){
        i = ptask_create_prio(telescope, PER, PRIO, NOW);
        if (i != -1) {
            printf("Telescope acquisition, %d created and activated\n", i);
            
        } else {
            allegro_exit();
            printf("Error in creating task!\n");
            exit(-1);
        }     
    }

    /* Crea gui */
    i = ptask_create_prio(gui, PER, PRIO, NOW);
    if (i != -1) {
        printf("gui, %d created and activated\n", i);
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    /* Crea elaboratori */
    i = ptask_create_prio(compute_result, PER, PRIO, NOW);
    if (i != -1) {
        printf("compute, %d created and activated\n", i);
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    do {
        k = 0;
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }
        
    } while (k != KEY_ESC);

    
    allegro_exit();
    sleep(1);
    
    return 0;
}