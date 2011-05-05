#include <sys/socket.h>

#define PORT       10010
#define QUEUE_SIZE 5

static inline void die(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0)
    die("opening socket");
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);
  if (bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    die("bind");

  listen(server, QUEUE_SIZE);

  struct sockaddr_in client_addr;
  int client = accept(server,
                      (struct sockaddr*)&server_addr,
                      sizeof(client_addr));
  if (client < 0)
    die("accept");

  int command[2] = {0,0};
  n = read(client,command,sizeof(command));
  if (n < 0)
    die("read");
  command[0] = ntohi(command[0]);
  command[1] = ntohi(command[1]);

  switch (command[0]) {
    case WACD_SET:
      printf("SET      %d\n", command[1]);
      break;
    case WACD_GET:
      printf("GET      %d\n", command[1]);
      break;
    case WACD_MOMENT:
      printf("MOMENT   %d\n", command[1]);
      break;
    case WACD_GO:
      printf("GO       %d\n", command[1]);
      break;
    case WACD_STOP:
      printf("STOP     %d\n", command[1]);
      break;
    case WACD_GOTO:
      printf("GOTO     %d\n", command[1]);
      break;
    case WACD_SHUTDOWN:
      printf("SHUTDOWN %d\n", command[1]);
      break;
    default:
      die("unknown command");
  }
  return 0;
}
