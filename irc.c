#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

#define IRC_CHANNEL "#secrettestchannel"
#define IRC_ADDRESS "irc.quakenet.org"
#define IRC_PORT "6667"
#define OWNER "Bingo"
#define NICKNAME "SuchImpressive"
#define NO_DATA_TIMEOUT 300
#define RECONNECT_INTERVAL 30
#define STARTING 0
#define WORKING 1
#define RESTARTING 3
#define ACTIVE 4

volatile int status = STARTING;

int con_socket;

time_t IRC_LAST_ACTIVITY = 0;
time_t IRC_RECONNECTION_ACTIVITY = 0;

pthread_mutex_t count_mutex;

void irc_reconnect()
{
  time_t present, delta;   
  time(&present);
  
  delta = present - IRC_RECONNECTION_ACTIVITY;

  if (delta >= RECONNECT_INTERVAL) 
  {
    printf(".:. IRC -> Connection to (%s) failed, reconnecting .:.\n", IRC_ADDRESS);
    pthread_mutex_lock(&count_mutex);
    status = RESTARTING;
    pthread_mutex_unlock(&count_mutex);
    time(&IRC_RECONNECTION_ACTIVITY);
  }
}

int socket_read(int sock, void *buffer, int len)
{
  int n;
  unsigned char *buf = buffer;

  n = read(sock, buf, len);
  
  if (n == -1 && errno == EAGAIN) // Resource temporarily unavailable
    return -EAGAIN;

  if (n == -1)
    return -1;

  return n;
}

void receive_message(void* arg)
{
  int irc_socket = *((int *) arg);
  char buffer[512];
  char nick[30];
  char body[256];
  char trash[288];
  while (socket_read(irc_socket, buffer, 512) > 0) 
  {
    //fputs(buffer, stdout);
    
    if (!strncmp(buffer, "PING ", 5)) 
    {
      buffer[1] = 'O'; //replace char 'I' on 'O' inside 'PING'
      send(irc_socket, buffer, strlen(buffer), 0); //send 'PONG'
    }
    
    if (strstr(buffer, "PRIVMSG #") != NULL) 
    {
      if (sscanf(buffer,"%c%[^!]!%[^:]:%s",trash, nick, trash, body) > 0)
      {
        printf("From %s: %s\n", nick, body);

      }
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

int socket_write(int sock, void *buffer, int len) 
{
  int n_sent;
  int n_left = len;
  char *buf = buffer;

  while (n_left > 0) 
  {
    n_sent = write(sock, buf, n_left);
    if (n_sent < 0)
    {
      if (errno == EINTR) // Interrupted function call
      { 
        continue;
      }

      return -1;
    }

    n_left -= n_sent;
    buf += n_sent; // Advance buffer pointer to the next unsent bytes
  }
  return len;
}

void send_message(void* arg)
{
  int irc_socket = *((int *) arg);
  char buffer[512];
  char message[1024];
  
  while (1) 
  {
    if (fgets(message, sizeof message, stdin)) 
    {
      sprintf(buffer, "PRIVMSG %s :%s\r\n", IRC_CHANNEL, message);

      if (socket_write(irc_socket, buffer, strlen(buffer)) < 0)
      {
        pthread_mutex_lock(&count_mutex);
        status = RESTARTING;
        pthread_mutex_unlock(&count_mutex);
      }
    }

    sleep(0);
  }
}

void send_static_message(int sock, char *message)
{
  if (socket_write(sock, message, strlen(message)) < 0)
  {
    pthread_mutex_lock(&count_mutex);
    status = RESTARTING;
    pthread_mutex_unlock(&count_mutex);
  }
}

void irc_timer()
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

int send_user_info(socket)
{
  char buffer[512];
  sprintf(buffer, "USER %s 0 * :%s\r\n", NICKNAME, OWNER);
  send_static_message(socket, buffer);
}

int send_nickname(socket)
{
  char buffer[512];
  sprintf(buffer, "NICK %s\r\n", NICKNAME);
  send_static_message(socket, buffer);
}

int irc_login(socket)
{
  printf(".:. Registration process... .:.\n");
  send_user_info(socket);
  send_nickname(socket);
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
    pthread_mutex_lock(&count_mutex);
    status = RESTARTING;
    pthread_mutex_unlock(&count_mutex);
  } 
  
  sleep(5);
  irc_login(con_socket);
}


int main(int argc, char *argv[])
{
  pthread_t receiver;
  pthread_t sender;
  pthread_t timer;
  
  pthread_mutex_init(&count_mutex, NULL);

  while (1) {
    if (status == STARTING) 
    {
      irc_cycle();
      status = WORKING;
    }
    
    if (status == WORKING) 
    {
      if (pthread_create (&receiver, NULL, (void *) &receive_message, &con_socket) != 0) 
      {
        fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
      }

      if (pthread_create (&sender, NULL, (void *) &send_message, &con_socket) != 0)
      {
        fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
      }

      if (pthread_create (&timer, NULL, (void *) &irc_timer, NULL) != 0)
      {
        fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
      }

      status = ACTIVE;
    }

    if (status == RESTARTING) 
    {
      pthread_cancel(receiver);
      pthread_cancel(sender);
      pthread_cancel(timer);

      status = STARTING;
      close(con_socket);
    }

    sleep(0);
  }

pthread_join(receiver, NULL);
pthread_join(sender, NULL);
pthread_join(timer, NULL);

pthread_mutex_destroy(&count_mutex);

return 0;
}
