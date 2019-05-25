#include "../lib/observatory.h"

void gui(){
    int i;
    int x, y;
    int x1, y1;
    int x2, y2;
    BITMAP *buffer;
    char s[12];
    char nl[16];

    buffer = create_bitmap(XWIN,YWIN);
    while(1){
        pthread_mutex_lock(&mutex);

        clear_to_color(buffer, BGC);
        line(buffer, 0, YWIN - LINE, XWIN, YWIN - LINE, 14); 

        // Scrivo il pianeta sul buffer.
        x = planet_x - OBS_SHAPE/2;
        y = planet_y - OBS_SHAPE/2;
        stretch_sprite(buffer, planet_img, x, y, 300, 300);
        fprintf(stderr, "planet x: %d, y: %d\n", x, y);
        // Realizzo il cielo

        clear_to_color(sky, BGC);
        stretch_sprite(sky, planet_img, x, y, 300, 300);

        // Scrivo telescopi sul buffer.
        for(i = 0; i < N; i++){
            x1 = 1 + i * (BASE - BORDER);
            y1 = YWIN - LINE + BORDER;
            x2 = (BASE - BORDER) * (i + 1); 
            y2 = y1 + BASE - BORDER - 1;

            textout_centre_ex(buffer, font, "Telescope", (x1 + x2) / 2,
                                YWIN - LINE + 2, 100, 0);

            //if(centered[i] != 1)
            stretch_sprite(buffer, tel.observation[i], x1, y1, x2 - x1, y2 - y1);

            rect(buffer, tel.x_pred[i], tel.y_pred[i], tel.x_pred[i]+ 2, tel.y_pred[i]+2, 40);

            rect(buffer, x1, y1, x2, y2, 100);

            if((mouse_x > x1) && (mouse_x < x2)){
                if((mouse_y > y1) && (mouse_y < y2)){
                    // Scrivo observation window sul buffer.
                    rect(buffer, tel.x_obs[i] - OBS_SHAPE/2, tel.y_obs[i] - OBS_SHAPE/2,
                            tel.x_obs[i] + OBS_SHAPE/2, tel.y_obs[i] + OBS_SHAPE/2, 14);
                }
            }
        }

        if(tel.elaborated == 1){
            x1 = 1 + N * (BASE - BORDER);
            y1 = YWIN - LINE + BORDER;
            x2 = (BASE - BORDER) * (N + 1); 
            y2 = y1 + BASE - BORDER - 1;
            textout_centre_ex(buffer, font, "Result:", (x1 + x2) / 2, YWIN - LINE + 2,
                      14, 0);
            stretch_sprite(buffer, result, x1, y1, x2 - x1, y2 - y1);
            rect(buffer, x1 + 1, y1, x2, y2, 14);

            //textout_centre_ex(buffer, font, "Result:", XWIN/2 - TBLOCK, YWIN/2, 0, 1);
        }

        /* Interfaccia di modifica dei parametri */
        rectfill(buffer, XDIAG, YDIAG, XDIAG + WDIAG, YDIAG+HDIAG, BGC);
        rect(buffer, XDIAG, YDIAG, XDIAG + WDIAG - 1, YDIAG + HDIAG, 15);

        for(i = 0; i < N; i++){
            sprintf(s, "Telescope %d", i+1);
            textout_ex(buffer, font, s, XDIAG + BORDER, YDIAG + TBLOCK*(i) + BORDER,
                      15, 0);
            sprintf(nl, "noise level: %d", tel.noise_level[i]);
            textout_ex(buffer, font, nl, XDIAG + BORDER*2, YDIAG + TBLOCK*(i) + BORDER*2,
                      15, 0);
        }

        // mostro il mouse
        show_mouse(buffer);

        blit(buffer, screen, 0,0,0,0,XWIN,YWIN);
        
        pthread_mutex_unlock(&mutex);
        ptask_wait_for_period();
    }
}