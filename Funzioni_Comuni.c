#include "richiesta.h"
char* getTime(){
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	char* curr_time = asctime(localtime(&rawtime));
	return curr_time;
}

int get_random(int min, int max) {
	int num = rand() % max + min; 
	return num; 
}
