#include "../lib/observatory.h"

//_____________________________________________________________________________
//_____________________________________________________________________________
//
//				The main function.
//_____________________________________________________________________________
//_____________________________________________________________________________

/**
 * Creates tasks and manages the initialization and the shutdown
 * of this program.
 */
int main(void){	
	init();

	create_tasks();

	wait_for_esc();

	pthread_mutex_lock(&mutex);

	allegro_exit();

	return 0;
}