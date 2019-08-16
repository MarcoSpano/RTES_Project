#include "../lib/observatory.h"

int main(void){
    int c;
    int i, k;

    finished = 0;
    init();
    clear_to_color(screen, BGC);
    
    start_ui();

    /* Creates planet */
    i = ptask_create_prio(planet, PER, PRIO, NOW);
    if (i != -1) {
        printf("Planet, %d created and activated\n", i);
        
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    /* Creates telescope motor */
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

    /* Creates gui */
    i = ptask_create_prio(gui, PER, PRIO, NOW);
    if (i != -1) {
        printf("gui, %d created and activated\n", i);
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    /* Creates elaboratori */
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

    pthread_mutex_lock(&mutex);
    
    allegro_exit();
    //sleep(1);
    
    return 0;
}