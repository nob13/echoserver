#pragma once

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(X) if(X < 0){ int err = errno; fprintf (stderr, "Error happened on line %d: %s\n", __LINE__, strerror (err)); return -1; }
#define BUFFER_SIZE 4096

inline bool isReadable (const char * x) {
	for (;*x != 0;x++){
		if (*x < 0x20) return false;
	}
	return true;
}

struct sockaddr_in;

class Server {
public:
	Server ();
	int init (int port);
	int run ();
private:
	int handleReceive ();

	int handleRequest (char * input, const char * humanSrcAddress, int port, const struct sockaddr_in *src);
	// globals
	int port;		///< Server port
	int epollfd;	///< Epoll fd
	int sock; 		///< listening socket
};
