#include "Server.h"

int main (int argc, char * argv[]) {
	int port;
	if(argc != 2) {
		printf ("Using default port 1234 (can also pass it as parameter)\n");
		port = 1234;
	} else {
		port = atoi (argv[1]);
	}
	Server server;
	CHECK (server.init(port));
	return server.run();
}

