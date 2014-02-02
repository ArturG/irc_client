#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> 

void* receive_message(void* arg) {
  int sockfd = *((int *) arg);
  char buffer[512];
  char *chan = "#testmeout";

  
  while (recv(sockfd, buffer, 512, 0) > 0) {
    fputs(buffer, stdout);
    
    if (!strncmp(buffer, "PING ", 5)) {
      buffer[1] = 'O';
      send(sockfd, buffer, strlen(buffer), 0);
    }

    if (!strncmp(strchr(buffer, ' ') + 1, "001", 3)) {
      sprintf(buffer, "JOIN %s\r\n", chan);
      send(sockfd, buffer, strlen(buffer), 0);
    }
  }
}

void* send_message(void* arg) {
  int sockfd = *((int *) arg);
  char buffer[512];
  char message[512];
  
  if (fgets(message, sizeof message, stdin)) {
    sprintf(buffer, "PRIVMSG #testmeout :%s\r\n", message);
    send(sockfd, buffer, strlen(buffer), 0);  
    printf("%s\n", buffer);
  }  
}

int connect_to_server(char *address, char *owner, char *nickname) {
  struct addrinfo hints;
  struct addrinfo *res;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  getaddrinfo(address, "6667", &hints, &res);
  
  int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  connect(sock, res->ai_addr, res->ai_addrlen);

  char buffer[512];
  
  sprintf(buffer, "USER %s 0 * :%s\r\n", nickname, owner);
  send(sock, buffer, strlen(buffer), 0);
  sprintf(buffer, "NICK %s\r\n", nickname);
  send(sock, buffer, strlen(buffer), 0);
  
  return sock;
}


int main(int argc, char *argv[]) {
  pthread_t receiver;
  pthread_t sender;

  int sock = connect_to_server(argv[1], argv[2], argv[3]);

  while (1) {
    pthread_create (&receiver, NULL, &receive_message, &sock);
    pthread_create (&sender, NULL, &send_message, &sock);
  }
}
