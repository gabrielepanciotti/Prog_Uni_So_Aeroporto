# Prog_Uni_So_Aeroporto
Progetto universitario per corso "Sistemi operativi"
Utilizzo del linguaggio C e comandi Unix per gestione piste di decollagio in un Aeroporto.
Utilizzo di processi padre/figlio (fork, execv), threads e memoria condivisa con mutua esclusione in attesa attiva(pthread, mutex, pthread_cond), pipe (mkfifo) e socket

Il programma funziona seguendo questi passi:
1. Viene avviato il processo Torre, questo prepara le n piste di decollaggio (n definito in define)
2. Crea la pipe (mkfifo) per lo scambio di notifiche con Hangar e i vari processi Aereo, si mette in ascolto (read) delle notifiche in arrivo fin quando non arriverà la notifica di fine decollo di tutti gli aerei. 
3. Viene avviato il processo Hangar, questo avvia gli x aerei (fork + execv) da far decollare (x definito in define)
4. Ogni processo figlio Aereo, una volta preparato, apre la pipe (open) creata dalla torre, invia una richiesta di decollo inviando la notifica tramite la pipe (write)
5. Si mette in attesa dell'autorizzazione da parte della torre, per farlo apre un socket(uno per ogni aereo) (bind) e si mette in ascolto (listen)
6. La torre in ascolto sulla pipe legge la notifica, riconosce che tipo di notifica è e da chi è stata inviata
7. Per ogni richiesta apre un nuovo thread (pthread_create) dove tramite accesso in mutua esclusione alla memoria condivisa tra thread (piste di decollo)(pthread_mutex_lock), ricerca se ci sono piste libere da assegnare all'aereo che ha inviato 
la richiesta da processare, se non ci sono piste disponibili, si mette in attesa (attiva) di un segnale da uno degli altri thread una volta aver liberato una pista (pthread_cond_wait).
8. Una volta trovata la pista libera la assegna all'aereo associato, si connette (connect) alla connessione socket dove è in ascolto l'aereo e gli invia un messaggio (write) di autorizzazione e uno con la pista su cui decollare
Nel frattempo il main thread non rimane in attesa della fine del thread figlio (pthread_detach) ma si rimette in attesa di nuove richieste di decollo da parte di altri aerei
9. La torre si mette quindi in attesa di una risposta dall'aereo, questo ricevuto il messaggio con il numero della pista, decolla occupando la pista per tot tempo,
a decollo avvenuto avvisa la torre inviandogli un messaggio tramite il socket e viene terminato il processo di quell'aereo
10. La torre riceve il messaggio di fine decollo, rimette disponibile la pista utilizzata e invia il segnale (pthread_cond_signal) di pista liberata a uno dei thread che sono in attesa di una pista, la richiesta è stato processata, termina il thread
11. L' hangar nel frattempo ha finito di avviare tutti i processi Aereo e si mette in attesa che terminano tutti (waitpid)
12. Una volta terminati tutti i processi Aereo, significa che tutti gli aerei sono decollati, l'hangar invia sulla pipe aperta inizialmente dalla torre, il messaggio di fine aerei e termina
13. La torre riceve il messaggio, informa su quanti aerei a fatto decollare e termina

Avrei potuto utilizzare semplicemente utilizzare una struttura client(aerei) / server(Torre), ma a scopo didattico, ho preferito utilizzare tutti gli strumenti a disposizione
Ho quindi spezzato la comunicazione in una prima parte tramite pipe dove gli aerei inviano la richiesta di decollo, in modo che ogni richiesta venga salvata nella FIFO.
Nella seconda parte poi ho invece utilizzato i socket per una comunicazione peer-to-peer tra ogni processo Aereo che richiede di decollare 
e il thread della Torre, assegnato alla gestione di quella specifica richiesta.
Avrei potuto utilizzare i semafori per la gestione della pista libera, ma dato che era più di una pista da gestire (n definibili) e dato che 
si può mettere in attesa solo di un semaforo alla volta, ho preferito utilizzare un semplice array condiviso.

A parte le attese random per la preparazione dell'aereo e per il decollo, non c'è nessun attesa passiva, ma tutte attive tramite i vari strumenti descritti prima 

Descrizione dettagliata progetto in file PDF

Per eseguire da Linux:
1. Scaricare l'intera cartella
2. Aprire bash sulla cartella
3. sudo chmod 777 Aeroporto.sh
4. ./Aeroporto.sh

Se viene interrotta un esecuzione o il codice non funziona correttamente, eseguire sempre da shell il comando:
make clean