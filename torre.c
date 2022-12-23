#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include "notifica.h"

char nome_processo[8] = "Torre";
bool pista_decollo[NUM_PISTE]; //true=libera, false=occupata

pthread_t tid[NUM_AEREI];
int n;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;


/* Ho pensato di usare i semafori ma dato che sono due piste, non saprei come metterlo in attesa di tutti e due i semafori in contemporanea,
per poi decrementare solo il primo che torna ad 1 (e annullare l'attesa dell'altro)
sem_t pista1;
sem_t pista2;
*/

//RETURN: -1: Nessuna pista libera ; i: Numero della pista libera
int ricercaPistaLibera(){ //Ricerca una pista di decollaggio libera
	for(int i=0;i<NUM_PISTE;i++){
		if(pista_decollo[i]==true){
			return i;
		}
	}
	return -1;
}

void fineAerei(int n_richieste){//Ricevuta notifica con pipe named da Hangar della fine degli aerei da far decollare, termina il programma
	char* curr_time;
	curr_time=getTime();
	printf("====================\n");
	printf("%s , %s : Notifica fine aerei da far decollare, terminazione programma \n", curr_time, nome_processo);
	printf("%s , %s : Aerei totali decollati %d \n", curr_time, nome_processo, n_richieste);
	printf("====================\n");
}

void *gestioneDecollo_Thread(void* num_aereo){
	int sfd;
	struct sockaddr_un sun;
	socklen_t sockT;
	char sBuffer[MAX_BUFFER_LEN];
	int iBytesRead;
	char* curr_time;

	int pista_libera=-1;
	char nome_aereo[16];
	char my_sock_path[32];
	char nome_processo_thread[32];
	
	//Tramite il numero dell'aereo ricava il nome e il socket con cui comunicare per la gestione del decollo
	sprintf(nome_aereo, "%s%ld", "Aereo", (long)num_aereo);
	sprintf(my_sock_path, "%s%ld", "/tmp/socket_Aereo", (long)num_aereo);
	sprintf(nome_processo_thread,"%s_%s",nome_processo,nome_aereo);

	//Ripete la ricerca fino a quando non trova una pista libera
	printf("====================\n");
	curr_time=getTime();
	printf("%s , %s : Ricerca pista libera...\n", curr_time, nome_processo_thread);
	while (pista_libera == -1){
		pthread_mutex_lock(&mutex); //Prende accesso sezione critica
		pista_libera = ricercaPistaLibera();
		
		if(pista_libera == -1){
			 //Rilascia accesso sezione critica per permettere ad altri thread di liberare una pista
			curr_time=getTime();
			printf("%s , %s : Nessuna pista disponibile, in attesa...\n", curr_time, nome_processo_thread);
			pthread_cond_wait(&condition, &mutex);
			curr_time=getTime();
			printf("%s , %s : Segnale arrivato, esce da attesa\n", curr_time, nome_processo_thread);
		}
		else{
			pista_decollo[pista_libera]=false; //Imposta pista trovata libera su occupata	
		}
		
		pthread_mutex_unlock(&mutex);
	} //Continua a ricercare una pista libera ogni secondo finche' non riesce ad autorizzare il decollo
	curr_time=getTime();
	printf("%s , %s : Pista %d assegnata per decollo ad aereo : %s\n", curr_time, nome_processo_thread, pista_libera, nome_aereo); 	
	
	//Invia autorizzazione decollo con la pista da cui decollare all'aereo richiedente aprendo una connesione Socket
	
	memset(&sun,'\0',sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, my_sock_path);
	if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		handle_error("socket");
	
	curr_time=getTime();
	printf("%s , %s : SOCK PATH : %s\n", curr_time, nome_processo_thread, my_sock_path);
	//Si connette tramite socket all'aereo, si � sicuri che va a connettersi con l'aereo che ha fatto la richiesta in quanto viene passato il percorso del sock
	// dell'aereo tramite la notifica di decollo
	//memset(&sun,'\0',sizeof(sun));
	if(connect(sfd,(struct sockaddr *)&sun, sizeof(sun)) == -1)
		handle_error("connect");
	
	//Invia ad aereo messaggio di autorizzazione decollo
	memset(sBuffer, 0, sizeof(sBuffer));
	strcpy(sBuffer,"Autorizzazione");
	if(write(sfd, sBuffer, strlen("Autorizzazione")) ==-1)
		handle_error("write");
	
	//Invia un altro messaggio con la pista dove decollare
	memset(sBuffer, 0, sizeof(sBuffer));
	sprintf(sBuffer,"%d",pista_libera);
	if(write(sfd, sBuffer, 1) ==-1)
		handle_error("write");
	
	//Si mette in attesa della notifica di fine decollo
	memset(sBuffer, 0, sizeof(sBuffer));
	if((iBytesRead = read(sfd, sBuffer, strlen("Decollo avvenuto"))) == -1)
		handle_error("read");
	
	if(strcmp(sBuffer,"Decollo avvenuto") == 0){
		pthread_mutex_lock(&mutex);
		//Imposta la pista scelta su occupata
		pista_decollo[pista_libera]=true;
		curr_time=getTime();
		printf("%s , %s : Invio segnale liberazione pista\n", curr_time, nome_processo_thread);
		pthread_cond_signal(&condition);
		pthread_mutex_unlock(&mutex);
		
		curr_time=getTime();
		printf("%s , %s : Notifica fine decollo aereo numero: %s\n", curr_time, nome_processo_thread, nome_aereo);
		printf("%s , %s : Pista liberata: %d\n", curr_time, nome_processo_thread,pista_libera);
	}
	else{
		printf("Ricevuto messaggio non previsto, chiusura connessione e thread");
	}
	
	printf("%s , %s : Chiusura Thread\n", curr_time, nome_processo_thread);
	printf("====================\n");
	//Chiusura socket con aereo per comunicazioni decollo aereo
	close(sfd);
	//Chiusura thread Torre per gestione decollo aereo
	pthread_exit(0);
}

//Concede l'autorizzazione ad un certo aereo di decollare e gli assegna una pista di decollo
void autorizzaDecollo(long num_aereo){ 
	//Crea un thread della torre per comunicare con l'aereo gli passa
	//come argomento il nome dell'aereo e il path delo socket a cui connetteri	
	int rc = 0;
	
	rc = pthread_create(&tid[n], NULL, &gestioneDecollo_Thread, (void *)num_aereo);
	//Fa il detach del thread cos� che il main thread pu� continuare a processare le altre richieste
	pthread_detach(tid[n]); 
	n++;
	
}

int processaRichieste(int fd){
	struct tNotifica stNotifica;
	char* curr_time;
	strcpy(stNotifica.tipo,"init");
	int n_richieste=0;
	int wstatus;
	int iReadCount;
	
	while(strcmp(stNotifica.tipo, "fineAerei") != 0) {
		curr_time=getTime();
		if((iReadCount = read(fd, &stNotifica, sizeof(stNotifica))) == -1) {
			perror("Torre: Errore in read");
			return 1;
		}
		else{
			if(strcmp(stNotifica.tipo, "richiestaDecollo") == 0){
				curr_time=getTime();
				printf("====================\n");
				printf("%s , %s : Richiesta n. %d | Tipo: %s \nId: Aereo%ld | Socket: /tmp/socket_Aereo%ld\n", curr_time, nome_processo, n_richieste, stNotifica.tipo, stNotifica.num, stNotifica.num);
				printf("====================\n");
				
				autorizzaDecollo(stNotifica.num);
				n_richieste++;
			}
		}
	}
	return n_richieste;	
}
void creaPiste(){
	char* curr_time;
	//Inizializzo tutte le piste a libere
	printf("====================\n");
	curr_time=getTime();
	printf("%s , %s : Numero piste di decollo aperte: %d \n", curr_time, nome_processo, NUM_PISTE);
	for (int i=0;i<NUM_PISTE;i++){
		pista_decollo[i]=true;	
	}
	printf("====================\n");
}

int main(int argc, char *argv[]){
	int n_richieste;
	int fd;
	n = 0;
	char* curr_time;
	
	creaPiste();
	
	//Creazione pipe per comunicazione con Torre(sia messaggio di termine da Hangar, sia richiesta di decollo da Aerei)
	curr_time=getTime();
	printf("%s , %s : In attesa notifiche decollo da aerei... \n", curr_time, nome_processo);
	mkfifo(MYPIPE, S_IRWXU);
	fd = open(MYPIPE, O_RDONLY);
	
	n_richieste=processaRichieste(fd);
	fineAerei(n_richieste);
	
	int rc = pthread_mutex_destroy(&mutex);
	close(fd);
	exit(0);
}
