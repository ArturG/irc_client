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
#define NO_DATA_TIMEOUT 300
#define RECONNECT_INTERVAL 30
#define STARTING 0
#define WORKING 1
#define RESTARTING 2

int status = STARTING;
int con_socket;
time_t IRC_LAST_ACTIVITY = 0;
time_t IRC_RECONNECTION_ACTIVITY = 0;

void irc_reconnect()
{
  time_t present, delta;   
  time(&present);
  
  delta = present - IRC_RECONNECTION_ACTIVITY;

  if (delta >= RECONNECT_INTERVAL) 
  {
    printf(".:. IRC -> Connection to (%s) failed, reconnecting .:.\n", IRC_ADDRESS);

    status = RESTARTING;
    time(&IRC_RECONNECTION_ACTIVITY);
  }
}

//TODO refactor
void* receive_message(void* arg)
{
  int irc_socket = *((int *) arg);
  char buffer[512];
  
  while (recv(irc_socket, buffer, 512, 0) > 0) 
  {
    fputs(buffer, stdout);
    
    if (!strncmp(buffer, "PING ", 5)) 
    {
      buffer[1] = 'O'; //replace char 'I' on 'O' inside 'PING'
      send(irc_socket, buffer, strlen(buffer), 0); //send 'PONG'
    }

    if (strstr(buffer, "PRIVMSG #") != NULL) 
    {
      printf("Received message: %s\n", buffer);
    }

    if (!strncmp(strchr(buffer, ' ') + 1, "001", 3)) 
    {
      sprintf(buffer, "JOIN %s\r\n", IRC_CHANNEL);
      send(irc_socket, buffer, strlen(buffer), 0);
      printf(".:. Connected to the channel %s .:. \n", IRC_CHANNEL);
      printf(".:. You can send messages directly through the console .:. \n");
    }

    time(&IRC_LAST_ACTIVITY);
  }

  // if while loop finished it means that we lost connection
  irc_reconnect();
}

//TODO refactor
void* send_message(void* arg)
{
  int irc_socket = *((int *) arg);
  int msg;
  char buffer[512];
  char message[512];
  
  if (fgets(message, sizeof message, stdin)) 
  {
    sprintf(buffer, "PRIVMSG %s :%s\r\n", IRC_CHANNEL, message);
    msg = send(irc_socket, buffer, strlen(buffer), 0); 
    if (msg == -1) 
    {
      printf("SUPERRR ERRRORRRR \n");
    } 
    printf("You said: %s\n", message);
  }  
}

void* irc_timer()
{
  sleep(30);

  time_t present, delta;
  time(&present);

  delta = present - IRC_LAST_ACTIVITY;

  if (delta >= NO_DATA_TIMEOUT) 
  {
    printf(".:. Timeout awaiting data from server .:. \n");
    irc_reconnect();
    time(&IRC_LAST_ACTIVITY);
  }
}

//TODO refactor
int irc_login(socket)
{
  char buffer[512];
  printf(".:. Registration process... .:.\n");
  sprintf(buffer, "USER %s 0 * :%s\r\n", NICKNAME, OWNER);
  send(socket, buffer, strlen(buffer), 0);
  sprintf(buffer, "NICK %s\r\n", NICKNAME);
  send(socket, buffer, strlen(buffer), 0);    
}

int get_socket(char* host, char* port)
{
  int rc;
  int irc_socket=-1;
  struct addrinfo hints;
  struct addrinfo *res=NULL;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  if ((rc = getaddrinfo(host, port, &hints, &res)) == 0 ) 
    {

     if ((irc_socket = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol)) > 0) 
       {

        if (connect(irc_socket, res -> ai_addr, res -> ai_addrlen) < 0)
         {
           fprintf(stderr, ".:. Couldn't connect .:.\n");
           close(irc_socket);
           irc_socket=-1;
         } 
         else
         {
           printf(".:. Connected! .:.\n");
         }
       }
     else fprintf(stderr, ".:. Couldn't get socket .:.\n");
    }
   
  else fprintf(stderr, ".:. getaddrinfo() error: %s .:.\n", gai_strerror(rc));

  if (res) freeaddrinfo(res);
  return irc_socket;
}

int irc_cycle()
{
  con_socket = get_socket(IRC_ADDRESS, IRC_PORT);
  
  if (con_socket < 0 ) 
  {
    printf(".:. Connection failed .:.\n");
    status = RESTARTING;
  } 
  
  sleep(5);
  irc_login(con_socket);
}


int main(int argc, char *argv[])
{
  pthread_t receiver;
  pthread_t sender;
  pthread_t timer;
  
  while (1) {
    if (status == STARTING) 
    {
      irc_cycle();
      status = WORKING;
    }
    
    if (status == WORKING) 
    {
      pthread_create (&receiver, NULL, &receive_message, &con_socket);
      pthread_create (&sender, NULL, &send_message, &con_socket);
      pthread_create (&timer, NULL, &irc_timer, NULL); 
    }

    if (status == RESTARTING) 
    {
      pthread_cancel(receiver);
      pthread_cancel(sender);
      pthread_cancel(timer);

      status = STARTING;
      close(con_socket);
    }
  }
}
