EXE = Torre Hangar
CC=gcc
#FLAGS= -pthread
all: $(EXE)
Torre: torre.c funzioni.c notifica.h
	$(CC) torre.c funzioni.c notifica.h -o Torre
Hangar: hangar.c funzioni.c notifica.h Aereo
	$(CC) hangar.c funzioni.c notifica.h -o Hangar
Aereo: aereo.c funzioni.c notifica.h Unlink
	$(CC) aereo.c funzioni.c notifica.h -o Aereo
Unlink: unlink_socket.c
	$(CC) unlink_socket.c -o Unlink
clean:
	@rm -f $(EXE)
