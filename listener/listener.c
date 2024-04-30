/* INCLUDES */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* PROTOTYPES */
void error(const char *err);

/* MAIN */
int main(int argc, char **argv) {
  /* INIT/DECLARE */
  int z,x;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  unsigned short portNumber;
  FILE *rStream = NULL;
  FILE *wStream = NULL;
  int s, c;
  char buf[4096];
  socklen_t addrlen;

  /* CHECK ARGS */
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <Port Number>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* SET PORT NUMBER */
  portNumber = atoi(argv[1]);

  /* CREATE TCP/IP SOCKET */
  if((s = socket(PF_INET, SOCK_STREAM, 0)) == -1 ) 
    error("[*] ERROR: Error, initializing socket");

  /* INIT SERVER STRUCT */
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET; // set ipv4
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // set network interfaces to all
  serverAddress.sin_port = htons(portNumber); // set port number
  
  /* BIND TO SERVER ADDRESS */
  if((z = bind(s, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) == -1)
    error("[*] ERROR: Error, binding socket to interface");

  /* LISTEN ON SOCKET */
  if((z = listen(s,10)) == -1) 
    error("[*] ERROR: Error, listening on socket");

  /* MAIN PROGRAM LOOP */
  for(;;) {

    /* ACCEPT CLIENT CONNECTION */
    addrlen = sizeof(clientAddress);
    if((c = accept(s, (struct sockaddr *)&clientAddress, &addrlen)) == -1)
       error("[*] ERROR: Error, client connection failed");
    
    /* READ CLIENT REQUESTS */
    if(!(rStream = fdopen(c, "r"))) {
      close(c);
      continue;
    }

    /* CREATE WRITE STREAM */
    if(!(wStream = fdopen(dup(c), "w"))) {
      fclose(rStream);
      continue;
    }

    /* SET STREAMS TO LINE BUFFERED MODE */
    setlinebuf(rStream);
    setlinebuf(wStream);

    /* PROCESS CLIENT REQUEST AND RESPONSE */


    
    /* CLOSE CLIENT CONNECTION */
    fclose(wStream);
    shutdown(fileno(rStream), SHUT_RDWR);
    fclose(rStream);

  }

  /* EXIT */
  exit(EXIT_SUCCESS);
}

/* FUNCTIONS */

/* ERROR */
void error(const char *err) {
  perror(err);
  exit(EXIT_FAILURE);
}
