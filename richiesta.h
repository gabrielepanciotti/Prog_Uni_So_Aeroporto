#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#ifndef MYHEADER_H_
#define MYHEADER_H_

char* getTime(); // only put declaration in headers
int get_random(int min, int max);

#define MYPIPE "/tmp/myfifo"
#define MY_SOCK_PATH "/tmp/my_socket"
#define LISTEN_BACKLOG 50
#define MAX_BUFFER_LEN	256
#define PORT			5152
#define NET_ADDRESS		"10.0.2.15"

#define handle_error(msg) \
   do { printf("errno = %d\n", errno); perror(msg); exit(EXIT_FAILURE); } while (0)

#define checkResults(string,val) \
	if(val){printf("Failed");exit(1);} 

#endif


//Struttura dati per richieste, primo campo per tipo richiesta, secondo per id mittente
struct tNotifica {
	char tipo[20];
	char id[10];
	char my_sock_path[30];
};


