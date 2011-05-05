/* Implementation of the Williams Analog Clock Daemon (server). */
/* Copyright 2011 Antal Spector-Zabusky and Andrew Lorenzen */
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

/* You can define any combination of the following three constants:
 *   - WACD_PRINT:    Prints the time at every clock tick.
 *   - WACD_PHYSICAL: Writes to the parallel port (requires root).
 *   - WACD_LOG:      Logs incoming messages and (if enabled) writes to the
 *                    parallel port.
 */

#define QUEUE_SIZE 5

#ifdef WACD_LOG
#define logf(...) printf(__VA_ARGS__)
#else
#define logf(...)
#endif

struct {
  int  time;
  int  moment;
  bool going;
} clock = { .time = 0, .moment = WACD_MIN_MOMENT, .going = false };

#ifdef WACD_PRINT
static inline void clock_print() {
  printf("%dh%02dm%02d [%d]\n",
         clock.time/(60*60),
         (clock.time/60) % 60,
         clock.time % 60,
         clock.time);
}
#else
static inline void clock_print() { }
#endif

#ifdef WACD_PHYSICAL
int port_file = -1;

static inline void clock_finish() {
  static uint8_t ticktock = 0;
  ++ticktock;
  logf("[ticktock = %d]\n", ticktock);
  lseek(port_file, WACD_PHYSICAL_PORT, SEEK_SET);
  write(port_file, &ticktock, sizeof(ticktock));
  ticktock %= 2;
  usleep(WACD_MIN_MOMENT*1000);
  logf("[rest]\n");
  int rest = 3;
  lseek(port_file, WACD_PHYSICAL_PORT, SEEK_SET);
  write(port_file, &rest, sizeof(rest));
  usleep((clock.moment - WACD_MIN_MOMENT)*1000);
}

static inline void port_setup() {
  if (setuid(0) < 0) {
    perror("Couldn't setuid to root");
    exit(EXIT_FAILURE);
  }
  port_file = open("/dev/port", O_WRONLY);
  if (port_file < 0) {
    perror("Couldn't access port file");
    exit(EXIT_FAILURE);
  }
}

static inline void port_cleanup() {
  close(port_file);
}
#else
static inline void clock_finish() {
  usleep(clock.moment*1000);
}

static inline void port_setup() { }
static inline void port_cleanup() { }
#endif

static inline void clock_tick() {
  if (clock.going) {
    clock.time = (clock.time + 1) % WACD_SECONDS_PER_CLOCK;
    clock_print();
    clock_finish();
  } else {
    usleep(clock.moment*1000);
  }
}

int main(int argc, char *argv[]) {
  int client = -1;
  int server = -1;
  int exit_status = EXIT_SUCCESS;
  
  port_setup();
  
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
      } else if (n > 0) {
        command[0] = ntohl(command[0]);
        command[1] = ntohl(command[1]);

        #define REPLY(msg) do {                       \
          int response = msg;                         \
          write(client, &response, sizeof(response)); \
        } while(0)
        switch (command[0]) {
          case WACD_SET:
            logf("SET      %d\n", command[1]);
            if (command[1] >= 0 && command[1] < WACD_SECONDS_PER_CLOCK) {
              clock.time = command[1];
              REPLY(WACD_STATUS_OK);
            } else {
              REPLY(WACD_STATUS_ERR);
            }
            break;
          case WACD_GET:
            logf("GET      %d\n", command[1]);
            REPLY(clock.time);
            break;
          case WACD_MOMENT:
            logf("MOMENT   %d\n", command[1]);
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
            logf("GO       %d\n", command[1]);
            clock.going = true;
            REPLY(WACD_STATUS_OK);
            break;
          case WACD_STOP:
            logf("STOP     %d\n", command[1]);
            clock.going = false;
            REPLY(WACD_STATUS_OK);
            break;
          case WACD_GOTO:
            logf("GOTO     %d\n", command[1]);
            int distance = command[1] - clock.time;
            if (distance >= 0 && distance < 60*60) {
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
            logf("FINISH   %d\n", command[1]);
            close(client);
            client = -1;
            // Doesn't reply
            break;
          case WACD_SHUTDOWN:
            logf("SHUTDOWN %d\n", command[1]);
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
  port_cleanup();
  if (client < 0) close(client);
  if (server < 0) close(server);
  return exit_status;
}
