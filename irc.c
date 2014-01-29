#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int sockfd;
  char buffer[512];
  char *bot_owner = "Arturr";
  char *nick = "Test";
  char *chan = "#help";
  struct addrinfo hints;
  struct addrinfo *res;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  getaddrinfo(argv[1], "6667", &hints, &res);
  
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  
  sprintf(buffer, "USER %s 0 * :%s\r\n", nick, bot_owner);
  send(sockfd, buffer, strlen(buffer), 0);
  sprintf(buffer, "NICK %s\r\n", nick);
  send(sockfd, buffer, strlen(buffer), 0);

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


