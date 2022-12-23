#include <sys/socket.h>
#include <sys/un.h>
#include "richiesta.h"
	
int main(int argc, char *argv[]){
	unlink("/tmp/socket_Aereo1");
	unlink("/tmp/socket_Aereo2");
	unlink("/tmp/socket_Aereo3");
	unlink("/tmp/socket_Aereo4");
	unlink("/tmp/socket_Aereo5");
	unlink("/tmp/socket_Aereo6");
	unlink("/tmp/socket_Aereo7");
	unlink("/tmp/socket_Aereo8");
	unlink("/tmp/socket_Aereo9");
	unlink("/tmp/socket_Aereo10");
	printf("Socket chiusi\n");
}
