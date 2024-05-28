//Keenan Grant and Caleb Murphy

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)",
        "<Server Address/Name> <Echo Word> [<Server Port/Service>]");

  char *server = argv[1];     // First arg: server address/name
  // Third arg (optional): server port/service
  char *service = (argc == 3) ? argv[2] : "echo";

  // Create a connected TCP socket
  int sock = SetupTCPClientSocket(server, service);
  if (sock < 0)
    DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");

  //Recieve List of files from server

  fputs("\nRequesting a list of files from the server ", stdout);
  
  struct sockaddr_in localAddr;
  socklen_t addrSize = sizeof(localAddr);
  if (getsockname(sock, (struct sockaddr *) &localAddr, &addrSize) < 0)
    DieWithSystemMessage("getsockname() failed");

  printf("%s", inet_ntoa(localAddr.sin_addr));

  fputs("\nList recieved.", stdout);
  // Receive the serialized list from the server
  char fileListStr[1024];
  ssize_t bytesRead = recv(sock, fileListStr, sizeof(fileListStr) - 1, 0);
  if (bytesRead == -1) {
      perror("recv");
      exit(EXIT_FAILURE);
  }

  // Null-terminate the received data to make it a valid string
  fileListStr[bytesRead] = '\0';

  // Split the newline-separated string into individual file names
  char *fileList[5]; // Assuming up to 5 files
  int numFiles = 0;
  char *token = strtok(fileListStr, "\n");
  while (token != NULL) {
      fileList[numFiles] = token;
      numFiles++;
      token = strtok(NULL, "\n");
  }

  // Print the list of files
  printf("\nUser, please select a file:\n");
  printf("1.  %s\n", fileList[0]);
  printf("2. %s\n", fileList[1]);
  printf("3. %s\n", fileList[2]);


  int userChoice = 0;
  printf("\n> ");
  scanf("%d", &userChoice);
  char bufferNum[2];
  bufferNum[0] = userChoice;

  ssize_t bytesSent = send(sock, bufferNum, sizeof(bufferNum), 0);
  if (bytesSent == -1) {
      perror("send");
      close(sock);
      exit(EXIT_FAILURE);
  }

  char nameOfFile[100];
  ssize_t bytesOfName = recv(sock, nameOfFile, sizeof(nameOfFile) - 1, 0);
  if (bytesOfName == -1) {
    perror("bytesOfName recv");
    exit(EXIT_FAILURE);
  }

  nameOfFile[bytesOfName] = '\0';

  printf("Requesting file \"%s\"\n", nameOfFile);

  printf("File received:\n");

  // Receive and store the file content from the server
  char receivedContent[1024]; // Assuming a maximum file size of 1024 bytes
  ssize_t bytesReceived = read(sock, receivedContent, sizeof(receivedContent));
  if (bytesReceived == -1) {
      perror("recv");
      close(sock);
      exit(EXIT_FAILURE);
  }

  printf("%s\n", receivedContent);

  printf("Goodbye!!!\n");

  fputc('\n', stdout); // Print a final linefeed

  close(sock);
  exit(0);
}