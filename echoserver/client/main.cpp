#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#define CHECK(X) if(X < 0){ int err = errno; fprintf (stderr, "Error happened on line %d: %s\n", __LINE__, strerror (err)); return -1; }

int main (int argc, char * argv[]) {
	const char * dstname = "127.0.0.1";
	int dstport = 1234;
	printf ("Usage %s server port\n", argv[0]);
	if (argc >= 2) {
		dstname = argv[1];
	}
	if (argc >= 3) {
		dstport = atoi (argv[2]);
	}

	// Creating socket
	int s = socket (AF_INET, SOCK_DGRAM, 0);


	// Source Address to bind on
	struct sockaddr_in src;
	memset (&src, 0, sizeof (sockaddr_in));
	src.sin_family = AF_INET;
	src.sin_port   = htons (6843);
	src.sin_addr.s_addr   = inet_addr ("0.0.0.0");
	CHECK (bind (s, (const struct sockaddr*) &src, sizeof (src)));
	printf ("Bound source address to 6843\n");

	// Destination Address
	struct sockaddr_in dst;
	memset (&dst, 0, sizeof (sockaddr_in));
	dst.sin_family = AF_INET;
	dst.sin_port   = htons (dstport);
	dst.sin_addr.s_addr   = inet_addr (dstname);
	printf ("Destination: %s:%d\n", dstname, dstport);

	// Sending
	const char * msg = "condata 0x12345678";
	CHECK (sendto (s, msg, strlen(msg), 0, (const struct sockaddr*) &dst, sizeof(dst)));
	printf ("Sent: %s\n", msg);

	// Waiting for an answer
	int epfd = epoll_create (1);
	struct epoll_event epollevent;
	epollevent.events = EPOLLIN;			// we want read on it
	epollevent.data.fd = s; 			   	// user set (we use the file descriptor)

	CHECK (epoll_ctl (epfd, EPOLL_CTL_ADD, s, &epollevent));

	while (true){
		int timeout = 5000;
		struct epoll_event changed[1];
		int n = epoll_wait (epfd, changed, 1, timeout);
		if (n == 0) {
			fprintf (stderr, "Received no message within %dms\n", timeout);
			continue;
		}
		char buffer [256];
		buffer[255] = 0;
		ssize_t length = read (changed[0].data.fd, buffer, 255);
		CHECK (length);
		if (length >= 0 && length < 255) buffer[length] = 0;
		printf ("Recv: %s\n", buffer);
		break;
	}

	close (s);
	return 0;
}

