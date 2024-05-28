//Keenan Grant and Caleb Murphy

#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <math.h>

// Handle error with user msg
void DieWithUserMessage(const char *msg, const char *detail);
// Handle error with sys msg
void DieWithSystemMessage(const char *msg);
// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);
// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);
// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);
// Handle new TCP client
void HandleTCPClient(int clntSocket);
// Create and connect a new TCP client socket
int SetupTCPClientSocket(const char *server, const char *service);
// Checks if packet is the same size on both ends
unsigned short checksum(void *b, int len);

enum sizeConstants {
  MAXSTRINGLENGTH = 128,
  BUFSIZE = 512,
};

int count = 0x7fffffff;
int port = 33333;
float ping_interval = 1.0;
size_t packet_size = 12;
bool print = true;
bool serverMode = false;
char* server_ip;
int numReceived = 0;
double rtt, min = -1, max = -1, avg = 0, total = 0;
int packTrans = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#endif // PRACTICAL_H_