#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "richiesta.h"

#define NUM_AEREI 10


char* nome_processo = "Hangar";
pid_t aerei_pid[10];
pid_t aerei[10];
pid_t pid;
int fd;

//Crea il numero di aerei richiesto, creando un processo Aereo per ognuno
void creazioneAerei(){
	char *curr_time;
	char *num = "a";
	int x = 0;
	for(int i=0;i<NUM_AEREI;i++){
		curr_time=getTime();
		printf("%s , %s : Creazione aereo numero: %d\n", curr_time, nome_processo, i+1);
		
		//Crea processo figlio aereo
		aerei[i]=fork();
		//pid = getpid();
		//aerei_pid[i] = pid;
		if (aerei[i] < 0) { /* error occurred */
			fprintf(stderr, "Fork Failed, errno = %d\n", errno);
			perror((const char *)NULL);
		}
		else if (aerei[i] == 0) { /* child process */
			//pid = getpid();
			//aerei_pid[i] = pid;
			//sprintf(nome_processo,"%s_%d",nome_processo,pid); 
			//Esegue Aereo.o con execv con argomento numero aereo, riprende con argv
			//sprintf(num,"%d",i+1);
			char *argv[] = {"./Aereo",NULL};
			//printf("argv1: %s\n",argv[1]);
			for (x=0; x<2; x++){
				printf("argv: %s\n", argv[x]);
			}
			execv(argv[0],argv);
		}
		else { /* parent process */
			pid = getpid();
			printf("Parent process: %d\n", pid);
			printf("Child process: %d\n", aerei[i]);
			//Attende 2 secondi per la prossima creazione
			sleep(2);
		}
	}
	curr_time=getTime();
	printf("%s , %s : Creazione aerei terminata,attesa terminazione decollo aerei \n", curr_time, nome_processo);	
}

void notificaFineAerei(){ //Attende la fine di tutti i processi Aereo e notifica la Torre della fine degli aerei
	int wstatus;
	char *curr_time;
	//Istanzio la notifica
	struct tNotifica stNotifica;
	for (int x=0; x<10; x++){
		printf("Pid aerei: %d\n",aerei[x]);
	}
	for(int i=0;i<NUM_AEREI;i++){
		printf("Pid Aereo in attesa: %d ; ", aerei[i]);
		//?? Perchè non si blocca qua?? (Non parte execv)?
		waitpid(aerei[i], &wstatus, 0);
		printf("Stato: %i\n", wstatus);
		do {
			curr_time=getTime();
			printf("%s , %s : Aereo %d ancora in decollo\n",curr_time, nome_processo, i+1);
			sleep(10);
		} while ((WIFEXITED(wstatus)) == 0);
	}
	curr_time=getTime();
	printf("%s , %s : Terminato decollo aerei, notifica chiusura \n", curr_time, nome_processo);
	
	//Setto i parametri della notifica
	strcpy(stNotifica.tipo,"fineAerei");
	strcpy(stNotifica.id,nome_processo);
	//Apro pipe per inviare notifica fine aerei
    fd = open(MYPIPE, O_WRONLY);
	//Invia messaggio di fine decolli nella pipe e la chiude
    write(fd, &stNotifica, sizeof(stNotifica));
    close(fd);
}

int main(int argc, char *argv[]){
	//Creazione pipe per comunicazione con Torre(sia messaggio di termine da Hangar, sia richiesta di decollo da Aerei)
	mkfifo(MYPIPE, S_IRWXU);
	creazioneAerei();
	notificaFineAerei();
	
	exit(0);
}
