#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include "wacd.h"

#define QUEUE_SIZE 5

struct {
  int  time;
  int  moment;
  bool going;
} clock = { .time = 0, .moment = WACD_MIN_MOMENT, .going = false };

static inline void clock_tick() {
  if (clock.going) {
    // 60 sec/min * 60 min/hr * 12 hr/clock
    clock.time = (clock.time + 1) % WACD_SECONDS_PER_CLOCK;
    #ifdef WACD_PRINT
    printf("%dh%02dm%02d [%d]\n",
           clock.time/(60*60),
           (clock.time/60) % 60,
           clock.time % 60,
           clock.time);
    #endif
    #ifdef WACD_PHYSICAL
    /* :TODO: Unimplemented */
    #endif
  }
  usleep(clock.moment*1000);
}

int main(int argc, char *argv[]) {
  int client = -1;
  int server = -1;
  int exit_status = EXIT_SUCCESS;
  
  server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) {
    perror("Couldn't open socket");
    exit_status = EXIT_FAILURE;
    goto exit;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(WACD_PORT);
  int opt_true = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt_true, sizeof(opt_true));
  if (bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Couldn't bind");
    exit_status = EXIT_FAILURE;
    goto exit;
  }
  int server_flags = fcntl(server, F_GETFL);
  fcntl(server, F_SETFL, server_flags|O_NONBLOCK);
  listen(server, QUEUE_SIZE);
  
  while (true) {
    if (client < 0) {
      struct sockaddr_in client_addr;
      socklen_t client_sz = sizeof(client_addr);
      // client is scoped to all of main so it can be closed more easily
      client = accept(server, (struct sockaddr*)&server_addr, &client_sz);
      if (client < 0 && errno != EWOULDBLOCK) {
        perror("Couldn't accept client");
      } else {
        int client_flags = fcntl(client, F_GETFL);
        fcntl(client, F_SETFL, client_flags|O_NONBLOCK);
      }
    }

    if (client >= 0) {
      uint32_t command[2] = {0,0};
      int n = read(client,command,sizeof(command));
      if (n < 0 && errno != EWOULDBLOCK) {
        perror("Couldn't read from client");
      } else if (errno != EWOULDBLOCK) {
        command[0] = ntohl(command[0]);
        command[1] = ntohl(command[1]);

        #define REPLY(msg) do {                       \
          int response = msg;                         \
          write(client, &response, sizeof(response)); \
        } while(0)
        switch (command[0]) {
          case WACD_SET:
            printf("SET      %d\n", command[1]);
            if (command[1] >= 0 && command[1] < WACD_SECONDS_PER_CLOCK) {
              clock.time = command[1];
              REPLY(WACD_STATUS_OK);
            } else {
              REPLY(WACD_STATUS_ERR);
            }
            break;
          case WACD_GET:
            printf("GET      %d\n", command[1]);
            REPLY(clock.time);
            break;
          case WACD_MOMENT:
            printf("MOMENT   %d\n", command[1]);
            if (command[1] >= WACD_MIN_MOMENT) {
              // The maximum moment is the client's responsibility; they can
              // wait for as long as they want between commands.
              clock.moment = command[1];
              REPLY(WACD_STATUS_OK);
            } else {
              REPLY(WACD_STATUS_ERR);
            }
            break;
          case WACD_GO:
            printf("GO       %d\n", command[1]);
            clock.going = true;
            REPLY(WACD_STATUS_OK);
            break;
          case WACD_STOP:
            printf("STOP     %d\n", command[1]);
            clock.going = false;
            REPLY(WACD_STATUS_OK);
            break;
          case WACD_GOTO:
            printf("GOTO     %d\n", command[1]);
            int distance = command[1] - clock.time;
            if (distance >=0 && distance < 60*60) {
              clock.going = true;
              while (clock.time < command[1])
                clock_tick();
              clock.going = false;
              REPLY(WACD_STATUS_OK);
            } else {
              REPLY(WACD_STATUS_ERR);
            }
            break;
          case WACD_FINISH:
            printf("FINISH   %d\n", command[1]);
            close(client);
            client = -1;
            // Doesn't reply
            break;
          case WACD_SHUTDOWN:
            printf("SHUTDOWN %d\n", command[1]);
            exit_status = EXIT_SUCCESS;
            // Doesn't reply
            goto exit;
          default:
            fprintf(stderr, "Unknown command (%x,%d).\n",
                    command[0], command[1]);
            REPLY(WACD_STATUS_ERR);
        }
        #undef REPLY
      }
    }
    clock_tick();
  }
  
  exit:
  if (client < 0) close(client);
  if (server < 0) close(server);
  perror("");
  return exit_status;
}
