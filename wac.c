#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "wac.h"

int net;

int wac_init(char *hostname) {
  int n;
  struct sockaddr_in saddr; /* an ip(4) socket address */
  struct hostent *server;
  char buffer[2];
  int port = DEFAULT_PORT;

  /* get server address */
  server = gethostbyname(hostname);
  if(server == NULL) {
    perror("gethostbyname");
    exit(0);
  }

  /* Create appropriate TCP stream socket */
  net = socket(AF_INET, SOCK_STREAM, 0);
  if(net < 0) {
    perror("socket");
    exit(1);
  }

  /* Connect to host on port */
  memset(&saddr,0,sizeof(saddr));
  saddr.sin_family = AF_INET;
  /* copy the host address to the socket */
  bcopy(server->h_addr, &(saddr.sin_addr.s_addr), server->h_length);
  saddr.sin_port = htons(port);
  if(connect(net,(struct sockaddr *)&saddr,sizeof(saddr)) < 0) {
    perror("connect");
    exit(1);
  }
  return 0;
}

int wac_go() {
  int buffer[2];
  buffer[0] = WACD_GO;
  buffer[1] = 0;
  n = write(net,buffer,2);
  if(n < 0) {
    perror("write");
    exit(1);
  }
}

int wac_finish() {
  /* close things down */
  close(net);
  return 0;
}

int main(int argc, char **argv) {
  char *hostname = (argc>1)?argv[1]:"localhost";
  wac_init(hostname);
  wac_go();
  wac_finish();
}
