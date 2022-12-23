#include "notifica.h"

char* getTime(){
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	char *curr_time_long = asctime(localtime(&rawtime));
	char* curr_time = curr_time_long + 4;
	curr_time[strlen(curr_time)-1] = '\0';
	return curr_time;
}

int get_random(int min, int max) {
	srand(time(NULL));
	int num = min + rand() % (max+1 - min);
	return num; 
}
