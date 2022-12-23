EXE = Torre Hangar
CLEAN = Torre Hangar Aereo Unlink
CC=gcc

all: $(EXE)
Torre: torre.c funzioni.c notifica.h
	$(CC) torre.c funzioni.c notifica.h -o Torre
Hangar: hangar.c funzioni.c notifica.h Aereo
	$(CC) hangar.c funzioni.c notifica.h -o Hangar
Aereo: aereo.c funzioni.c notifica.h 
	$(CC) aereo.c funzioni.c notifica.h -o Aereo
	
clean:
	@rm -f $(CLEAN)
	$(CC) unlink_socket.c -o Unlink
	./Unlink
