#include <stdlib.h>
#include <stdio.h> 
#include <conio.h> 
#include <time.h>

#define MYPIPE "/tmp/myfifo"
#define MY_SOCK_PATH "/tmp/my_socket"
#define LISTEN_BACKLOG 50   /* The  backlog  argument defines the maximum length to which the queue of pending connections 
							   for sockfd may grow.  If a connection request arrives  when  the  queue  is
							   full,  the client may receive an error with an indication of ECONNREFUSED or, if the
							   underlying protocol supports retransmission, the request may be ignored  so  that  a
							   later reattempt at connection succeeds. */
#define MAX_BUFFER_LEN	256
#define PORT			5152
#define NET_ADDRESS		"10.0.2.15"

#define handle_error(msg) \
   do { printf("errno = %d\n", errno); perror(msg); exit(EXIT_FAILURE); } while (0)
 
//Struttura dati per richieste, primo campo per tipo richiesta, secondo per id mittente
struct tNotifica {
	char tipo[20];
	char id[10];
	char my_sock_path[30];
};

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
