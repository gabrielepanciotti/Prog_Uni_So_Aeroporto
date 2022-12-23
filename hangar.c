#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "notifica.h"

char* nome_processo = "Hangar";
pid_t aerei_pid[NUM_AEREI];
pid_t aerei[NUM_AEREI];
pid_t pid;
int fd;

//Crea il numero di aerei richiesto, creando un processo Aereo per ognuno
void creazioneAerei(){
	char *curr_time;
	char num[32];
	int x = 0;

	for(int i=0;i<NUM_AEREI;i++){
		curr_time=getTime();
		printf("%s , %s : Creazione aereo numero: %d\n", curr_time, nome_processo, i+1);
		
		//Crea processo figlio aereo
		aerei[i]=fork();
		if (aerei[i] < 0) { /* error occurred */
			fprintf(stderr, "Fork Failed, errno = %d\n", errno);
			perror((const char *)NULL);
		}
		else if (aerei[i] == 0) { /* child process */
			//Esegue Aereo.o con execv con argomento numero aereo, riprende con argv
			sprintf(num,"%d",i+1);
			char *argv[] = {"Aereo", num, NULL};
			execv("./Aereo", argv);
		}
		else { /* parent process */
			pid = getpid();
			//Attende 2 secondi per la prossima creazione
			sleep(2);
		}
	}
	curr_time=getTime();
	printf("%s , %s : Creazione aerei terminata,attesa terminazione decollo aerei \n", curr_time, nome_processo);	
}

void notificaFineAerei(int fd){ //Attende la fine di tutti i processi Aereo e notifica la Torre della fine degli aerei
	int wstatus;
	char *curr_time;
	
	//Istanzio la notifica
	struct tNotifica stNotifica;
	printf("====================\n");
	for(int i=0;i<NUM_AEREI;i++){
		waitpid(aerei[i], &wstatus, 0);
		if((WIFEXITED(wstatus)) != 0){
			curr_time=getTime();
			printf("%s , %s : Termine decollo aereo %d\n", curr_time, nome_processo, i+1);
		}
	}
	curr_time=getTime();
	printf("%s , %s : Terminato decollo aerei, notifica chiusura \n", curr_time, nome_processo);
	printf("====================\n");
	//Setto i parametri della notifica
	strcpy(stNotifica.tipo,"fineAerei");
	stNotifica.num = -1;
	//Invia messaggio di fine decolli nella pipe e la chiude
    	write(fd, &stNotifica, sizeof(stNotifica));
    	
}

int main(int argc, char *argv[]){
	int fd;
	//Apro pipe per inviare notifica fine aerei
    	fd = open(MYPIPE, O_WRONLY);
    	
	creazioneAerei();
	notificaFineAerei(fd);
	
	close(fd);
	return 0;
}
