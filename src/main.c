#include "../lib/observatory.h"

int main(void){	
	init();

	start_ui();	

	create_tasks();

	wait_for_esc();

	pthread_mutex_lock(&mutex);

	allegro_exit();

	return 0;
}