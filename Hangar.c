#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "richiesta.h"

#define NUM_AEREI 10


static const char NOME_PROCESSO[] = "Hangar";
pid_t aerei[];
pid_t pid;
int fd;
char *curr_time;

//Crea il numero di aerei richiesto, creando un processo Aereo per ognuno
void creazioneAerei(){ 
	for(int i=0;i<NUM_AEREI;i++){
		curr_time=getTime();
		printf("%s , %s : Creazione aereo numero: %d\n", curr_time, NOME_PROCESSO, i+1);
		
		//Crea processo figlio aereo
		aerei[i]=fork();
		if (aerei[i] < 0) { /* error occurred */
			fprintf(stderr, "Fork Failed, errno = %d\n", errno);
			perror((const char *)NULL);
		}
		else if (aerei[i] == 0) { /* child process */
			pid = getpid();
			
			//Esegue Aereo.c con execv con argomento numero aereo, riprende con argv
			char *num;
			itoa(num,i+1,10);
			char *argv[] = {"Aereo",num,NULL};
			execv("Aereo.c",argv);
		}
		else { /* parent process */
			//Attende 2 secondi per la prossima creazione
			sleep(2);
		}
	}
	curr_time=getTime();
	printf("%s , %s : Creazione aerei terminata,attesa terminazione decollo aerei \n", curr_time, NOME_PROCESSO);	
}

void notificaFineAerei(){ //Attende la fine di tutti i processi Aereo e notifica la Torre della fine degli aerei
	int wstatus;
	//Istanzio la notifica
	struct tNotifica stNotifica;
	
	for(int i=0;i<NUM_AEREI;i++){
		waitpid(aerei[i], &wstatus, 0);
		if(WIFEXITED(wstatus)) {
			curr_time=getTime();
			printf("%s , %s : Aereo %d decollato correttamente\n",curr_time, NOME_PROCESSO, i+1);
		}
	}
	curr_time=getTime();
	printf("%s , %s : Terminato decollo aerei, notifica chiusura \n", curr_time, NOME_PROCESSO);
	
	//Setto i parametri della notifica
	strcpy(stNotifica.tipo,"fineAerei");
	strcpy(stNotifica.id,NOME_PROCESSO);
	//Apro pipe per inviare notifica fine aerei
    fd = open(MYPIPE, O_WRONLY);
	//Invia messaggio di fine decolli nella pipe e la chiude
    write(fd, &stNotifica, sizeof(stNotifica));
    close(fd);
}

int main(int argc, char *argv[]){
	pid = getpid();
	//Creazione pipe per comunicazione con Torre(sia messaggio di termine da Hangar, sia richiesta di decollo da Aerei)
	mkfifo(MYPIPE, S_IRWXU);
	creazioneAerei();
	notificaFineAerei();
	
	exit(0);
}
