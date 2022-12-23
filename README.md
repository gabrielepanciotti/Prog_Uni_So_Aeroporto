# Prog_Uni_So_Aeroporto
Progetto universitario per corso "Sistemi operativi"
Utilizzo del linguaggio C e comandi Unix per gestione piste di decollagio in un Aeroporto.
Utilizzo di processi padre/figlio (fork, execv), threads e memoria condivisa con mutua esclusione in attesa attiva(pthread, mutex, pthread_cond), pipe (mkfifo) e socket

Il programma funziona seguendo questi passi:
1. Viene avviato il processo Torre, questo prepara le n piste di decollaggio (n definito in define)
2. Apre la pipe per lo scambio di notifiche con Hangar e i vari processi Aereo, si mette in ascolto delle notifiche in arrivo fin quando non arriverà la notifica di fine decollo di tutti gli aerei. 
3. Viene avviato il processo Hangar, questo avvia gli x aerei (processi figli + execv) da far decollare (x definito in define)
4. Ogni processo figlio Aereo, apre la pipe creata dalla Torre e una volta preparato, invia una richiesta di decollo inviando la notifica  ,
Descrizione dettagliata progetto in file PDF

Per eseguire scaricare tutto la cartella e avviare Aeroporto.sh
