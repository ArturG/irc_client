#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int sockfd;
  int i, l, sl, o = -1;
  char buffer[512];
  char buf[513];
  struct addrinfo hints;
  struct addrinfo *res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  getaddrinfo(argv[1], "6667", &hints, &res);
  
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  
  while ((sl = read(sockfd, buffer, 512))) {
    for (i = 0; i < sl; i++) {
      o++;
      buf[o] = buffer[i];
      if ((i > 0 && buffer[i] == '\n' && buffer[i - 1] == '\r') || o == 512) {
        buf[o + 1] = '\0';
        l = o;
        o = -1;       
        printf(">> %s", buf);
      }   
    }
  }

  freeaddrinfo(res);
}


