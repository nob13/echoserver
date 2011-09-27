#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/epoll.h>

Server::Server () {
	port = -1;
	epollfd = -1;
	sock = .1;
}

int Server::init (int port) {
	this->port = port;
	// Creating socket
	sock = socket (AF_INET, SOCK_DGRAM, 0);

	// Source Address to bind on
	struct sockaddr_in src;
	memset (&src, 0, sizeof (sockaddr_in));
	src.sin_family = AF_INET;
	src.sin_port   = htons (port);
	src.sin_addr.s_addr   = inet_addr ("0.0.0.0");
	CHECK (bind (sock, (const struct sockaddr*) &src, sizeof (src)));

	printf ("Bound to %d\n", port);
	return 0;
}

int Server::run () {
	// Waiting for data
	int epfd = epoll_create (1);
	struct epoll_event epollevent;
	epollevent.events = EPOLLIN;			// we want read on it
	epollevent.data.fd = sock; 			   	// user set (we use the file descriptor)
	CHECK (epoll_ctl (epfd, EPOLL_CTL_ADD, sock, &epollevent));


	struct epoll_event changed[1];
	int n;
	while ((n = epoll_wait (epfd, changed, 1, -1)) > 0){
		CHECK (handleReceive());
	}

	close (epollfd);
	close (sock);
	return 0;
}

static void print (const char * what, const char * buffer) {
	if (isReadable(buffer)){
		printf ("%s %s\n", what, buffer);
	} else {
		printf ("%s [unreadable]\n", what);
	}
}

static int checkTypeLength (int got, int requested) {
	if (got != requested){
		fprintf (stderr, "Length mismatch");
		return 1;
	}
	return 0;
}

int Server::handleReceive () {
	char input[BUFFER_SIZE];
	struct sockaddr_in from;

	socklen_t socklen = sizeof (from);
	ssize_t size = recvfrom (sock, input, BUFFER_SIZE - 1, 0, (struct sockaddr*) &from, &socklen);
	CHECK (checkTypeLength (socklen, sizeof(from)));
	if (size < 0) {
		int err = errno;
		fprintf (stderr, "Could not read from socket: %s\n", strerror (err));
		return 0; // continue
	}
	input[size] = 0;
	print ("Received: ", input);

	char humanSrcAddress [128];
	if (!inet_ntop (AF_INET, &from.sin_addr, humanSrcAddress, 128)){
		fprintf (stderr, "Could not decode address");
		return 1;
	}
	int port = ntohs (from.sin_port);
	printf ("Source: %s:%d\n", humanSrcAddress, port);
	return handleRequest (input, humanSrcAddress, port, &from);
}

int Server::handleRequest (char * input, const char * humanSrcAddress, int port, const struct sockaddr_in *from) {
	char * strtok_saveptr;
	char * cmd = strtok_r (input, " ", &strtok_saveptr);
	if (!cmd) return 0; // ignore
	if (strcmp (cmd, "condata") != 0){
		return 0; // also ignore, wrong init sequence
	}
	char * token = strtok_r (0, " ", &strtok_saveptr);
	if (!token) {
		return 0; // ignore, no token
	}
	// We have everything we want
	// check length
	if (strlen (token) +  strlen(humanSrcAddress) + /* portLength */ 5 + /*header*/ 64 > BUFFER_SIZE){
		return 0; // ignore to long
	}

	char output [BUFFER_SIZE];
	/*
	printf ("token: %s\n", token);
	printf ("humanSrcAddress: %s\n", humanSrcAddress);
	printf ("port: %d\n", port);
	*/
	snprintf (output, BUFFER_SIZE - 1, "condataReply %s %s %d", token, humanSrcAddress, port);
	output[BUFFER_SIZE-1] = 0;

	print ("Will send", output);
	ssize_t sent = sendto (sock, output, strlen(output), 0, (const struct sockaddr*) from, sizeof(struct sockaddr_in));
	if (sent < 0) {
		int err = errno;
		printf ("Problems sending it back %s!\n", strerror(err));
	}
	return 0;
}

