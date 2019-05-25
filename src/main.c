#include "../lib/observatory.h"

int main(void){
    int c;
    int i, k;
    int last_proc = 0;  /* last assigned processor  */
    int max_proc = ptask_getnumcores(); /* max number of procs  */
    char s[12];
    char nl[15];

    init();
    
    int x1, y1, x2, y2;
    do{
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }

        for(i = 0; i < N; i++){
            sprintf(s, "Telescope %d", i+1);
            textout_ex(screen, font, s, BORDER + i * XWIN/N, YWIN - LINE + BORDER, 15, 0);
            sprintf(nl, "noise level: %d", tel.noise_level[i]);
            textout_ex(screen, font, nl, BORDER * 2 + i * XWIN/N, YWIN -LINE + BORDER * 2,
                      15, 0);
            line(screen, (i+1) * XWIN/N, YWIN -LINE, (i+1) * XWIN/N, YWIN,
                      15);
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

    /* Crea pianeta */
    i = ptask_create_param(planet, &params);
    if (i != -1) {
        printf("Planet, %d created and activated\n", i);
        
    } else {
        allegro_exit();
        printf("Error in creating task!\n");
        exit(-1);
    }

    /* Crea telescope motor */
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
    /* Crea telescope */
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

    /* Crea gui */
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

    /* Crea elaboratori */
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

    do {
        k = 0;
        if (keypressed()) {
            c = readkey();
            k = c >> 8;
        }
        
    } while (k != KEY_ESC);

    allegro_exit();
    
    return 0;
}