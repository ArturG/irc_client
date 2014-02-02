#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> 

#define IRC_CHANNEL "#help"
#define IRC_ADDRESS "irc.quakenet.org"
#define OWNER "Bingo"
#define NICKNAME "SuchImpressive"

void* receive_message(void* arg) {
  int irc_socket = *((int *) arg);
  char buffer[512];

  while (recv(irc_socket, buffer, 512, 0) > 0) {
    //fputs(buffer, stdout);
    
    if (!strncmp(buffer, "PING ", 5)) {
      buffer[1] = 'O'; //replace char 'I' on 'O' inside 'PING'
      send(irc_socket, buffer, strlen(buffer), 0); //send 'PONG'
    }

    if (strstr(buffer, "PRIVMSG #") != NULL) {
      printf("Received message: %s\n", buffer);
    }

    if (!strncmp(strchr(buffer, ' ') + 1, "001", 3)) {
      sprintf(buffer, "JOIN %s\r\n", IRC_CHANNEL);
      send(irc_socket, buffer, strlen(buffer), 0);
      printf(".:. Connected to the channel %s .:. \n", IRC_CHANNEL);
      printf(".:. You can send messages directly through the console .:. \n");
    }
  }
}

void* send_message(void* arg) {
  int irc_socket = *((int *) arg);
  char buffer[512];
  char message[512];
  
  if (fgets(message, sizeof message, stdin)) {
    sprintf(buffer, "PRIVMSG %s :%s\r\n", IRC_CHANNEL, message);
    send(irc_socket, buffer, strlen(buffer), 0);  
    printf("You said: %s\n", message);
  }  
}

int connect_to_server() {
  struct addrinfo hints;
  struct addrinfo *res;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  getaddrinfo(IRC_ADDRESS, "6667", &hints, &res);
  
  int irc_socket = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
  connect(irc_socket, res -> ai_addr, res -> ai_addrlen);

  char buffer[512];
  
  sprintf(buffer, "USER %s 0 * :%s\r\n", NICKNAME, OWNER);
  send(irc_socket, buffer, strlen(buffer), 0);
  sprintf(buffer, "NICK %s\r\n", NICKNAME);
  send(irc_socket, buffer, strlen(buffer), 0);
  
  return irc_socket;
}


int main(int argc, char *argv[]) {
  pthread_t receiver;
  pthread_t sender;

  int irc_socket = connect_to_server();

  while (1) {
    pthread_create (&receiver, NULL, &receive_message, &irc_socket);
    pthread_create (&sender, NULL, &send_message, &irc_socket);
  }
}
