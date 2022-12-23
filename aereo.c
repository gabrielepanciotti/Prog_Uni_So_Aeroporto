#include <sys/socket.h>
#include <sys/un.h>
#include "notifica.h"

int sfd, cfd;

char sBuffer[MAX_BUFFER_LEN];
int iBytesRead;

char nome_processo[] = "Aereo";
long num_aereo;
char mysock[64];

void decolla(int num_pista){
	int tempo_decollo=get_random(5,15);
	char *curr_time;

	printf("====================\n");
	curr_time=getTime();
	printf("%s , %s : Inizio decollo aereo \nPista usata: %d \nTempo necessario decollo: %d \n", curr_time, nome_processo, num_pista, tempo_decollo);
	sleep(tempo_decollo); //Tempo random di decollo
	
	//Invia messaggio di decollo avvenuto tramite connessione Socket alla Torre
	memset(sBuffer, 0, sizeof(sBuffer));
	strcpy(sBuffer,"Decollo avvenuto");
	if(write(cfd, sBuffer, strlen("Decollo avvenuto")) ==-1)
		handle_error("write");
		
	curr_time=getTime();
	printf("%s , %s : Fine decollo aereo \n", curr_time, nome_processo);
	printf("====================\n");
}

//Ritorna il numero della pista su cui decollare
int attesaAutorizzazione(){
	struct sockaddr_un sun, sTorre;
	socklen_t sockT;
	int num_pista = -1;
	char *curr_time;

	strcpy(mysock,"/tmp/socket_");
	strcat(mysock,nome_processo);

	//Apre connessione Socket con Torre e si mette in attesa dell'autorizzazione da parte di questa
	if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		handle_error("socket");
	
	//Mette il socket visibile su /tmp/
	sockT = sizeof(sTorre);
	memset(&sTorre,'\0',sizeof(sTorre));
	memset(&sun,'\0',sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, mysock);

	printf("====================\n");
	curr_time=getTime();
	printf("%s , %s : Creazione Socket: %s\n", curr_time, nome_processo, mysock);
	
	if((bind(sfd,(struct sockaddr *)&sun, sizeof(sun))) == -1)
		handle_error("bind");
	
	//Si mette in ascolto di richieste di connessioni di un thread della Torre
	if(listen(sfd,LISTEN_BACKLOG) == -1)
		handle_error("listen");
	
	//Accetta la richiesta di connessione da parte di un thread della Torre
	if((cfd = accept(sfd,(struct sockaddr *)&sTorre, &sockT)) == -1)
		handle_error("accept");
	
	curr_time=getTime();
	printf("%s , %s : Attesa autorizzazione decollo da parte della Torre \n", curr_time, nome_processo);
	
	//Si mette in lettura, aspettando l'autorizzazione del decollo da parte di un thread della Torre
	memset(sBuffer, 0, sizeof(sBuffer));
	if((iBytesRead = read(cfd, sBuffer, strlen("Autorizzazione"))) == -1)
		handle_error("read");
		
	curr_time=getTime();
	printf("%s , %s : %s decollo da parte della Torre \n", curr_time, nome_processo, sBuffer);
	
	//Se riceve il messaggio di autorizzazione, si rimette in lettura per leggere su quale pista decollare
	if(strcmp(sBuffer,"Autorizzazione") == 0){
		memset(sBuffer, 0, sizeof(sBuffer));
		if((iBytesRead = read(cfd, sBuffer, 1)) == -1)
			handle_error("read");
		num_pista = atoi(sBuffer);
	}
	curr_time=getTime();
	printf("%s , %s : Numero pista per decollo: %d\n", curr_time, nome_processo, num_pista);
	printf("====================\n");
	return num_pista;
}

void richiestaDecollo(int fd){
	//Crea la notifica con il tipo richiesta di decollo e il nome del processo che lo richiede
	char *curr_time;
	
	struct tNotifica stNotifica;
	strcpy(stNotifica.tipo,"richiestaDecollo");
	stNotifica.num = num_aereo;
	
	//Scrive la richiesta nella pipe e la chiude
	write(fd, &stNotifica , sizeof(stNotifica));

	curr_time=getTime();	
	printf("%s , %s : Invio richiesta decollo a torre \n", curr_time, nome_processo);
}

void preparazioneAereo(){ //Tempo di attesa per l aereo per la preparazione, determinato random
	char *curr_time;
	int tempo_preparazione=get_random(3,8);

	curr_time=getTime();
	printf("%s , %s : Inizio preparazione aereo, tempo necessario: %d\n", curr_time, nome_processo, tempo_preparazione);
	sleep(tempo_preparazione);
}

int main(int argc, char *argv[]){
	int fd;
	int num_pista;
	char *curr_time;

	num_aereo=(long)atoi(argv[1]);
	strcat(nome_processo,argv[1]);

	preparazioneAereo();

	fd = open(MYPIPE, O_WRONLY);
	richiestaDecollo(fd);
	
	//La funzione ritorna il numero della pista su cui decollare, -1 se non ha ricevuto l'autorizzazione
	num_pista = attesaAutorizzazione();
	if(num_pista != -1){
    		decolla(num_pista);
	}
	else{
		printf("%s , %s : Decollo non autorizzato \n", curr_time, nome_processo);
	}
	
	//Chiude Pipe, connessione socket e termina processo
	close(fd);
	close(sfd);
	close(cfd);
	unlink(mysock);
	return 0;
}
