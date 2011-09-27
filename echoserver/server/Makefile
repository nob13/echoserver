CFLAGS=-g

echoserver: main.cpp Server.o
	g++ ${CFLAGS} main.cpp Server.o -o echoserver
	
Server.o: Server.h Server.cpp
	g++ ${CGLAGS} -c Server.cpp -o Server.o
	
