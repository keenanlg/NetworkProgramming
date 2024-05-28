//Keenan Grant and Caleb Murphy

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "Practical.h"

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int SetupTCPServerSocket(const char *service) {
  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  int servSock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Create a TCP socket
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Socket creation failed; try next address

    // Bind to the local address and set socket to listen
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0)) {
      // Print local address of socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() failed");
    
      break;       // Bind and listen successful
    }

    close(servSock);  // Close and try again
    servSock = -1;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}

int AcceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Client address
  // Set length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  // clntSock is connected to a client!

  return clntSock;
}

void HandleTCPClient(int clntSocket) {
  char buffer[BUFSIZE]; // Buffer for echo string

  fputs("\nRecieved file list request from ", stdout);

  struct sockaddr_in localAddr;
  socklen_t addrSize = sizeof(localAddr);
  if (getsockname(clntSocket, (struct sockaddr *) &localAddr, &addrSize) < 0)
    DieWithSystemMessage("getsockname() failed");

  printf("%s", inet_ntoa(localAddr.sin_addr));

  fputs("\nSending list of files\n", stdout);

  char *fileList[] = {"song.txt", "poem.txt", "quote.txt"};
  int numFiles = 3;

  // Serialize the list into a newline-separated string
  char fileListStr[1024];
  for (int i = 0; i < numFiles; i++) {
      strcat(fileListStr, fileList[i]);
      if (i < numFiles - 1) {
          strcat(fileListStr, "\n");
      }
  }

  // Send the serialized list to the client
  ssize_t bytesSent = send(clntSocket, fileListStr, strlen(fileListStr), 0);
  if (bytesSent == -1) {
      perror("send");
      exit(EXIT_FAILURE);
  }

  int userChoice;
  char bufferNum[2];
  read(clntSocket, bufferNum, sizeof(bufferNum));

  userChoice = bufferNum[0];

  printf("Received request for file \"%s\"\n", fileList[userChoice - 1]);

  // Send the name of the file
  ssize_t fileNameSent = send(clntSocket, fileList[userChoice - 1], strlen(fileList[userChoice - 1]), 0);

  FILE* file = fopen(fileList[userChoice -1], "rb");
  if (file == NULL) {
      perror("fopen FILE* file");
      close(clntSocket);
      exit(EXIT_FAILURE);
  }

  char fileContent[1024];
  size_t bytesRead = fread(fileContent, 1, sizeof(fileContent), file);

  fclose(file);

  fputs("Sending file to the client\n", stdout);

  bytesSent = send(clntSocket, fileContent, bytesRead, 0);
  if (bytesSent == -1) {
      perror("send");
      close(clntSocket);
      exit(EXIT_FAILURE);
  }

  fputs("File sent\n\nGoodbye!!!\n", stdout);
  
  fputc('\n', stdout); // Print a final linefeed


  close(clntSocket); // Close client socket
}