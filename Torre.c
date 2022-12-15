#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "richiesta.h"

#define NUM_PISTE 2

static char nome_processo[] = "Torre";
bool pista_decollo[NUM_PISTE]; //true=libera, false=occupata

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

char* curr_time;

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
	curr_time=getTime();
	printf("%s , %s : Notifica fine aerei da far decollare, terminazione programma \n", curr_time, nome_processo);
	printf("%s , %s : Aerei totali decollati %d \n", curr_time, nome_processo, n_richieste);
	
	exit(0);
}

void gestionDecollo_Thread(char* args){
	/* RICERCA FUORI DA SEZIONE CRITICA
	do {
		pista_libera = ricercaPistaLibera();
		if(pista_libera == -1){
			//Attende 1 secondo in attesa che si liberino piste
			sleep(1);
		}
	}while (pista_libera == -1); //Continua a ricercare una pista libera ogni secondo finche' non riesce ad autorizzare il decollo
	
	while (1) { 
		if (pista_decollo[pista_libera]){
			pthread_mutex_lock(&mutex);
			//Imposta la pista scelta su occupata
			pista_decollo[pista_libera]=false;
			pthread_mutex_unlock(&mutex);
			printf("%s , %s : Pista %d assegnata per decollo ad aereo : %s\n", time, nome_processo_thread, pista_libera, nome_aereo); 
			break;	
		}
		else{
			printf("%s , %s : Errore assegnazione pista, la pista %d è occupata attualmente\n", time, nome_processo_thread, pista_libera);
			printf("Attende 3 secondi e prova a riassegnare la pista\n");
			sleep(3); 
		}
	}
	*/
	
	/*2 opzione: RICERCA DELLA PISTA LIBERA DA MAIN E THREAD IN ATTESA DI CONDIZIONE, COME COMUNICO A THREAD QUALE PISTA?
	int rc;
	char nome_aereo[20];
	int pista_libera;
	nome_aereo = strtok(args, ";")
	pista_libera = atoi(strtok(NULL, ";"))
	pthread_mutex_lock(&mutex);
	rc = pthread_cond_wait(&condition, &mutex);
	if (rc) {
		pista_decollo[pista_libera]=false;
		pthread_mutex_unlock(&mutex);
		printf("%s , %s : Pista %d assegnata per decollo ad aereo : %s\n", time, nome_processo, pista_libera, nome_aereo);
	}
	*/
	
	//1 opzione: RICERCA DELLA PISTA LIBERA DA THREAD DENTRO SEZIONE CRITICA, ?? LA RICERCA LA FACCIO DENTRO LA SEZIONE CRITICA O FUORI??
	int pista_libera=-1;
	
	//Scompongo la stringa args per riprendere i due parametri
	nome_aereo = strtok(args, ";")
	my_sock_path = strtok(NULL, ";")

	char nome_processo_thread[20];
	sprintf(nome_processo_thread,"%s_%s",nome_processo,nome_aereo)
	
	
	//Ripete la ricerca fino a quando non trova una pista libera
	do {
		pthread_mutex_lock(&mutex); //Prende accesso sezione critica
		pista_libera = ricercaPistaLibera();
		if(pista_libera == -1){
			pthread_mutex_unlock(&mutex); //Rilascia accesso sezione critica per permettere ad altri thread di liberare una pista
			//Attende 1 secondo in attesa che si liberino piste
			sleep(1);
		}
	}while (pista_libera == -1); //Continua a ricercare una pista libera ogni secondo finche' non riesce ad autorizzare il decollo
	
	pthread_mutex_lock(&mutex); //Prende accesso sezione critica
	pista_decollo[pista_libera]=false; //Imposta pista trovata libera su occupata
	pthread_mutex_unlock(&mutex); //Rilascia accesso sezione critica
	
	curr_time=getTime();
	printf("%s , %s : Pista %d assegnata per decollo ad aereo : %s\n", curr_time, nome_processo_thread, pista_libera, nome_aereo); 	
	
	//Invia autorizzazione decollo con la pista da cui decollare all'aereo richiedente aprendo una connesione Socket
	int sfd;
	struct sockaddr_un sun, sAereo;
	socklen_t sockT;
	char cBuffer[MAX_BUFFER_LEN];
	
	if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		handle_error("socket");
	
	//Si connette tramite socket all'aereo, si è sicuri che va a connettersi con l'aereo che ha fatto la richiesta in quanto viene passato il percorso del sock
	// dell'aereo tramite la notifica di decollo
	memset(&sun,'\0',sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, my_sock_path);
	if(connect(sfd,(struct sockaddr *)&sun, sizeof(sun)) == -1))
		handle_error("bind");
	
	//Invia ad aereo messaggio di autorizzazione decollo
	strcpy(sBuffer,"Autorizzazione");
	if(write(sfd, sBuffer, strlen(sBuffer)) ==-1)
		handle_error("write")
	
	//Invia un altro messaggio con la pista dove decollare
	char n_pista[4];
	itoa(pista_libera,n_pista,10);	
	strcpy(sBuffer,n_pista);
	if(write(sfd, sBuffer, strlen(sBuffer)) ==-1)
		handle_error("write")
	
	//Si mette in attesa della notifica di fine decollo
	memset(sBuffer, 0, sizeof(sBuffer));
	if((iBytesRead = read(sfd, sBuffer, sizeof(sBuffer))) == -1)
		handle_error("read");
	if(strcmp(sBuffer,"Decollo avvenuto") == 0){
		pthread_mutex_lock(&mutex);
		//Imposta la pista scelta su occupata
		pista_decollo[pista_libera]=true;
		pthread_mutex_unlock(&mutex);
		
		curr_time=getTime();
		printf("%s , %s : Notifica fine decollo aereo numero: %s\n", time, nome_processo_thread, nome_aereo);
		printf("%s , %s : Pista liberata: %d\n", time, nome_processo_thread, pista_decollo[pista_libera]);
	}
	else{
		printf("Ricevuto messaggio non previsto, chiusura connessione e thread")
	}
	
	//Chiusura socket con aereo per comunicazioni decollo aereo
	close(sfd);
	//Chiusura thread Torre per gestione decollo aereo
	pthread_exit((0);
}

//Concede l'autorizzazione ad un certo aereo di decollare e gli assegna una pista di decollo
void autorizzaDecollo(char* nome_aereo, char* sock_path){ 
	//Crea un thread della torre per comunicare con l'aereo
	pthread_t tid;
	int rc = 0;
	
	char* parametri;
	sprintf(parametri, "%s;%s", nome_aereo, sock_path)
	
	rc = pthread_create(&tid, NULL, &gestioneDecollo_Thread, (void *)&parametri);
	checkResults("pthread_create()\n", rc); 
	
	printf("Created thread %d\n", i + 1);
	pthread_detach(tid);
	
	/*2 opzione: RICERCA DELLA PISTA LIBERA DA MAIN E THREAD IN ATTESA DI CONDIZIONE, COME LO INFORMO DELLA PISTA LIBERA?
	int pista_libera = -1;
	
	rc = pthread_mutex_lock(&mutex);  
    checkResults("pthread_mutex_lock()\n", rc);
    do {
		pista_libera = ricercaPistaLibera();
		if(pista_libera == -1){
			//Attende 1 secondo in attesa che si liberino piste
			sleep(1);
		}
	}while (pista_libera == -1); //Continua a ricercare una pista libera ogni secondo finche' non riesce ad autorizzare il decollo
	
	char* time=getTime();
    printf("%s , %s : Pista scelta per decollo: %d\n", time, nome_processo, pista_libera);
    rc = pthread_cond_signal(&condition); //Avvisa il thread in attesa della pista libera
    if (rc) {
        pthread_mutex_unlock(&mutex);
        printf("Producer: Failed to wake up consumer, rc=%d\n", rc);
        exit(1);
    }
    
    time=getTime();
    printf("%s , %s : Invio pista decollo ad aereo numero: %s\n", time, nome_processo, nome_aereo);
    rc = pthread_mutex_unlock(&mutex);
    checkResults("pthread_mutex_lock()\n",rc);
    */	
}

int processaRichieste(){
	struct tNotifica stNotifica;
	//Apre pipe per richieste di decollo da gli aerei
	fd = open(MYPIPE, O_RDONLY);
	
	stNotifica.tipo = '\0';
	int n_richieste=0;
	int wstatus;
	
	while(strcmp(stNotifica.tipo, "fineAerei") == 0) {
		if((iReadCount = read(fd, &stNotifica, sizeof(stNotifica)) == -1) {
			perror("Torre: Errore in read");
			return 1;
		}
		else{
			if(strcmp(stNotifica.tipo, "richiestaDecollo") == 0){
				curr_time=getTime();
				printf(("%s , %s : Richiesta n. %d \nTipo: %s \nId mittente: %s \n", curr_time, nome_processo, n_richieste, stNotifica.tipo, stNotifica.id);
				autorizzaDecollo(stNotifica.id,stNotifica.my_sock_path);
				/* Implementazione precedente creando un nuovo processo
				richiesta[n_richieste]=fork();
				if (richiesta[n_richieste] < 0) { // error occurred 
					fprintf(stderr, "Fork Failed");
					break;
				}
				else if (richiesta[n_richieste] == 0) { // child process
					//Cambia il nome del processo in esecuzione aggiungendo la parola figlio e il suo numero
					sprintf(nome_figlio, "_figlio%d", n_richieste);
					strcat(nome_processo,nome_figlio);
					autorizzaDecollo(stNotifica.id,stNotifica.my_sock_path);
				}
				*/
				n_richieste++;
			}
		}	
	}
	close(fd);
	/* Aspetto che tutti i figli di Torre siano terminati, ma non serve basta la notifica di fine aerei (con previa verifica di termine processo Aerei)
	for(int i=0;i<n_richieste;i++){
		waitpid(richiesta[i], &wstatus, 0);
		while(!(WIFEXITED(wstatus)))
			sleep(1);s
	}
	*/
	return n_richieste;	
}

int main(int argc, char *argv[]){
	
	//Inizializzo tutte le piste a libere
	for(int i=0;i<NUM_PISTE;i++){
		pista_decollo[i]=true;	
	}
	int n_richieste=processaRichieste();
	fineAerei(n_richieste);
}
