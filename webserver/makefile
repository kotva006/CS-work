CC = gcc
CFLAGS = -D_REENTRANT
LDFLAGS = -lpthread

all: web_server_http

web_server_http: server.o util.o
	${CC} ${LDFLAGS} -o web_server_http server.o util.o

util.o:	util.c

server.o: server.c

clean:
	rm -f *.o web_server_http *~

