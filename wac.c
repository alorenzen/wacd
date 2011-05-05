#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "wac.h"
#include "wacd.h"
int net;

int wac_init(char *hostname) {
  struct sockaddr_in saddr; /* an ip(4) socket address */
  struct hostent *server;

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
    return 1;
  }

  /* Connect to host on port */
  memset(&saddr,0,sizeof(saddr));
  saddr.sin_family = AF_INET;
  /* copy the host address to the socket */
  bcopy(server->h_addr_list[0], &(saddr.sin_addr.s_addr), server->h_length);
  saddr.sin_port = htons(WACD_PORT);
  if(connect(net,(struct sockaddr *)&saddr,sizeof(saddr)) < 0) {
    perror("connect");
    return 1;
  }
  return 0;
}

int wac_send(int buffer[2]) {
  buffer[0] = htonl(buffer[0]);
  buffer[1] = htonl(buffer[1]);
  int n = write(net,buffer,sizeof(buffer));
  if(n < 0) {
    perror("write");
    return 1;
  }
  int return_buffer[1];
  n = read(net,return_buffer,sizeof(return_buffer));
  if(n < 0) {
    perror("read");
    return 1;
  }
  return ntohl(return_buffer[0]);
}

int wac_set(int secs) {
  int buffer[2] = { WACD_SET, secs };
  return wac_send(buffer);
}

int wac_get(void) {
  int buffer[2] = { WACD_GET, 0 };
  return wac_send(buffer);
}

int wac_moment(int msecs) {
  int buffer[2] = { WACD_MOMENT, msecs };
  return wac_send(buffer);
}

int wac_go() {
  int buffer[2] = { WACD_GO, 0 };
  return wac_send(buffer);
}

int wac_stop() {
  int buffer[2] = { WACD_STOP, 0 };
  return wac_send(buffer);
}

int wac_goto(int time) {
  int buffer[2] = { WACD_GOTO, time };
  return wac_send(buffer);
}

int wac_finish() {
  int buffer[2] = { WACD_FINISH, 0 };
  int response = wac_send(buffer);
  return close(net) < 0 ? WACD_STATUS_ERR : response;
}

int wac_shutdown() {
  int buffer[2] = { WACD_SHUTDOWN, 0 };
  int return_value = wac_send(buffer);
  wac_finish();
  return return_value;
}
