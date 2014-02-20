#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define IRC_CHANNEL "#testmeout"
#define IRC_ADDRESS "irc.quakenet.org"
#define IRC_PORT "6667"
#define OWNER "Bingo"
#define NICKNAME "SomeRandomNickname"

/* Defines time in which IRC client will timeout 
 * if no data is received */
#define NO_DATA_TIMEOUT 300
#define RECONNECT_INTERVAL 60

time_t IRC_LAST_ACTIVITY = 0;   // time of receiving last full line data from IRC server

void* receive_message(void* arg) {
  int irc_socket = *((int *) arg);
  char buffer[512];
  
  while (recv(irc_socket, buffer, 512, 0) > 0) {
    fputs(buffer, stdout);
    
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

    time(&IRC_LAST_ACTIVITY);
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


void irc_reconnect() {
  time_t present;
   
  time(&present);
  
  printf("IRC -> Connection to (%s) failed, reconnecting. \n", IRC_ADDRESS);

  //TODO
}

void* irc_timer() {
  sleep(30);

  time_t present, delta;
  time(&present);

  delta = present - IRC_LAST_ACTIVITY;

  if (delta >= NO_DATA_TIMEOUT) {
    printf(".:. Timeout awaiting data from server .:. \n");
    irc_reconnect();
    time(&IRC_LAST_ACTIVITY);
  }
}

int irc_login(socket) {
  char buffer[512];
  //this block should be moved from here
  sprintf(buffer, "USER %s 0 * :%s\r\n", NICKNAME, OWNER);
  send(socket, buffer, strlen(buffer), 0);
  sprintf(buffer, "NICK %s\r\n", NICKNAME);
  send(socket, buffer, strlen(buffer), 0);    
}

int get_socket(char* host, char* port) {
  int rc;
  int irc_socket;
  struct addrinfo hints;
  struct addrinfo *res;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  if ((rc = getaddrinfo(host, port, &hints, &res)) != 0 ) {
    fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(rc));
    return -1;
  }

  irc_socket = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
  
  if (irc_socket < 0) {
    fprintf(stderr, "Couldn't get socket.\n");
    goto error;
  }

  if (connect(irc_socket, res -> ai_addr, res -> ai_addrlen) < 0) {
    fprintf(stderr, "Couldn't connect.\n");
    goto error;
  }
  
  freeaddrinfo(res);
  return irc_socket;

error:
  freeaddrinfo(res);
  return -1;
}


int main(int argc, char *argv[]) {
  pthread_t receiver;
  pthread_t sender;
  pthread_t timer;
  
  int irc_socket = get_socket(IRC_ADDRESS, IRC_PORT);

  if (irc_socket < 0 ) {
    printf("Connection failed.\n");
    goto exit_err;
  } 

  irc_login(irc_socket);
  
  while (1) {
    pthread_create (&receiver, NULL, &receive_message, &irc_socket);
    pthread_create (&sender, NULL, &send_message, &irc_socket);
    pthread_create (&timer, NULL, &irc_timer, NULL); 
  }

exit_err:
  exit(1);
}
