/* csci4061 F2013 Assignment 5
* section: 004
* date: 12/10/2013
* names: Ryan , Benjamin
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

int s_fd, sock, len;  //Server Socket

void init(int port) {
  //Initial data to store socket information
  struct sockaddr_in s_addr;
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(port);
  s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int en = 1;
  //Create Socket fd
  if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating server socket");
    exit(1);
  }
  //Set socket fd to be reusable
  if (setsockopt(s_fd, SOL_SOCKET,SO_REUSEADDR, (char*)&en, sizeof(int)) < 0){
    perror("Error setting port options");
    exit(1);
  }
  //Bind socket
  if (bind(s_fd, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
    perror("Error binding to socket");
    exit(1);
  }
  //Create queue in socket of size 1
  if (listen(s_fd,1) < 0) {
    perror("Error starting listen");
    exit(1);
  }
}


int accept_connection(void) {
  struct sockaddr addr;
  len = sizeof(addr);
  sock = accept(s_fd,(struct sockaddr *)&addr,(socklen_t*)&len);
  //Create socket to return to client
  if (sock < 0) {
    perror("Error accepting socket");
    return -1;
  }
  return sock;
}

int get_request(int fd, char *filename) {
  FILE *fp;
  fp = fdopen(fd, "r");
  if(fp == NULL){
    perror("Error opening up stream using fd:\n");
    return -1;
  }
  //Stream in filename, ignoring first string
  if (fscanf(fp, "%*s %s", filename) == EOF ||
      strstr(filename, "//") != NULL ||
      strstr(filename, "..") != NULL) {
        fprintf(stderr, "Found bad request\n");
        return -1;
  }
  //We don't close fp because it will also close fd
  return 0;
}

int return_result(int fd, char *content_type, char *buf, int numbytes) {
  //Try the write the return, and if any step returns an error prints error
  char str[10];
  if(write(fd, "HTTP/1.1 200 OK\n", 16) < 0 ||
     write(fd, "Content-Type: ",14) < 0 ||
     write(fd, content_type, strlen(content_type)) < 0 ||
     write(fd, "\nContent-Length: ", 17) < 0 ||
     sprintf(str, "%d",numbytes) < 0 ||
     write(fd, str, strlen(str)) < 0 || 
     write(fd, "\nConnection: Close\n\n", 20) < 0 ||
     send(fd, buf, numbytes, 0) < 0 ||
     write(fd, "\n", 1) < 0 ||
     close(fd) < 0) {

    perror("Error writing to socket");
    return -1;

  }
  return 0;
}

int return_error(int fd, char *buf) {
  int temp = htonl((uint32_t)strlen(buf));
  if(write(fd,"HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: ",63)< 0|| 
     temp < 0 ||
     write(fd, &temp, sizeof(temp)) < 0 ||
     write(fd, "\nConnection: Close\n\n", 20) < 0 ||
     write(fd, buf, strlen(buf)) < 0 ||
     write(fd, "\n", 1) < 0 ||
     close(fd) < 0) {

    perror("Error writing to socket");
    return -1;
  }
  return 0;
}
