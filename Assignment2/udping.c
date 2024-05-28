/**
 * Caleb Murphy & Keenan Grant
 * CPSC 3600 Sec 001
 * 10/24/2023
*/

#include "Practical.h"

void signal_hdnler (int signo) {
    if (!serverMode) {
        avg = total / count;
        fprintf(stdout, "\n%d packets transmitted, %d received, %.0lf%% packet loss, time %.3lf ms"
                , packTrans, numReceived, (100.00 - (((double)numReceived / (double)packTrans) * 100.00)), total);
        printf("\nrtt min/avg/max = %.3f/%.3f/%.3f msec\n", min, avg, max);
        exit (0);
    }
    printf("\n");
    exit(0);
}

void* sender(void* arg) {
    struct timespec startTime, endTime;
    char *server = server_ip;
    char *packet = arg;
    pthread_mutex_unlock(&mutex);

    // Third arg (optional): server port/service
    char servPort[MAXSTRINGLENGTH] = "";
    sprintf(servPort, "%d", port);

    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;                   // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;             // Any address family
    // For the following fields, a zero value means "don't care"
    addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
    addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

    for (int i = 0; i < count; i++) {
        // Get address(es)
        struct addrinfo *servAddr; // List of server addresses
        int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
        if (rtnVal != 0)
            DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

        // Create a datagram/UDP socket
        int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
            servAddr->ai_protocol); // Socket descriptor for client
        if (sock < 0)
            DieWithSystemMessage("socket() failed");
        
        // Send the string to the server
        clock_gettime(CLOCK_REALTIME, &startTime);
        ssize_t numBytes = sendto(sock, packet, strlen(packet), 0,
            servAddr->ai_addr, servAddr->ai_addrlen);
        packTrans++;
        if (numBytes < 0)
            DieWithSystemMessage("sendto() failed");
        else if (numBytes != strlen(packet))
            DieWithUserMessage("sendto() error", "sent unexpected number of bytes");

        // Receive a response
        struct sockaddr_storage fromAddr; // Source address of server
        // Set length of from address structure (in-out parameter)
        socklen_t fromAddrLen = sizeof(fromAddr);
        char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
        numBytes = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
            (struct sockaddr *) &fromAddr, &fromAddrLen);
        if (numBytes < 0)
            DieWithSystemMessage("recvfrom() failed");
        else if (numBytes != strlen(packet))
            DieWithUserMessage("recvfrom() error", "received unexpected number of bytes");

        // Verify reception from expected source
        if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr *) &fromAddr))
            DieWithUserMessage("recvfrom()", "received a packet from unknown source");
        else
            numReceived++;
        
        clock_gettime(CLOCK_REALTIME, &endTime);
        rtt = (double)(endTime.tv_sec - startTime.tv_sec) * 1000.0 + (double)((endTime.tv_nsec / 1000.0) - (startTime.tv_nsec / 1000.0)) / 1000.0;
        if (min == -1 || rtt < min) {
            min = rtt;
        }
        if (max == -1 || rtt > max) {
            max = rtt;
        }
        total += rtt;
        freeaddrinfo(servAddr);

        buffer[packet_size] = '\0';     // Null-terminate received data
        if (print == true) {
            fprintf(stdout, "%5d\t%ld\t%.3lf\n", i + 1, packet_size, rtt);
        }
        
        close(sock);
    }
    if (print == false) {
        printf("\n**********\n");
    }
    avg = total / count;
    fprintf(stdout, "\n%d packets transmitted, %d received, %d%% packet loss, time %.3lf ms"
            , count, numReceived, 100 - ((numReceived / count) * 100), total);
    printf("\nrtt min/avg/max = %.3f/%.3f/%.3f msec\n", min, avg, max);
    exit(0);
    return NULL;
}

void* receiver(void* arg) {
    char service[MAXSTRINGLENGTH] = ""; // First arg:  local port/service
    sprintf(service, "%d", port);
    // Construct the server address structure
    struct addrinfo addrCriteria;                   // Criteria for address
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;             // Any address family
    addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
    addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

    struct addrinfo *servAddr; // List of server addresses
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

    // Create socket for incoming connections
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
        servAddr->ai_protocol);
    if (sock < 0)
    DieWithSystemMessage("socket() failed");

    // Bind to the local address
    if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    DieWithSystemMessage("bind() failed");

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    struct timespec abs_time;
    for (;;) { // Run forever
        struct sockaddr_storage clntAddr; // Client address
        // Set Length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Block until receive message from a client
        char buffer[MAXSTRINGLENGTH]; // I/O buffer

        // Size of received message
        ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
            (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (numBytesRcvd < 0)
        DieWithSystemMessage("recvfrom() failed");

        // Wait until it's time to send the next ping
        pthread_mutex_lock(&mutex);
        clock_gettime(CLOCK_REALTIME, &abs_time);

        abs_time.tv_sec += ping_interval; 
        pthread_cond_timedwait(&cond, &mutex, &abs_time);

        pthread_mutex_unlock(&mutex);

        // Send received datagram back to the client
        ssize_t numBytesSent = sendto(sock, buffer, numBytesRcvd, 0,
            (struct sockaddr *) &clntAddr, sizeof(clntAddr));
        numReceived++;
        if (numBytesSent < 0) {
             DieWithSystemMessage("sendto() failed)");
        }
        else if (numBytesSent != numBytesRcvd) {
            DieWithUserMessage("sendto()", "sent unexpected number of bytes");
        }
    }
    // NOT REACHED
    close(sock);
}

int main(int argc, char *argv[]) {
    int opt;
    pthread_t sender_thread, receiver_thread;

    signal(SIGINT, signal_hdnler);

    while ((opt = getopt(argc, argv, ":c:i:p:s:nS")) != -1) {
        switch(opt) {
            case 'S':
                serverMode = true;
                break;
            case 'c':
                count = atoi(optarg);
                if (!serverMode) {
                    fprintf(stderr, "\n%-9s\t%12d\n", "Count", count);
                }
                break;
            case 'i':
                pthread_mutex_lock(&mutex);
                ping_interval = atof(optarg);
                pthread_mutex_unlock(&mutex);
                if (!serverMode) {
                    fprintf(stderr, "%-9s\t%12.3f\n", "Interval", ping_interval);
                }
                break;
            case 'p':
                port = atoi(optarg);
                if (!serverMode) {
                    fprintf(stderr, "%-9s\t%12d\n", "Port", port);
                }
                break;
            case 's':
                packet_size = atoi(optarg);
                if (!serverMode) {
                    fprintf(stderr, "%-9s\t%12ld\n", "Size", packet_size);
                }
                break;
            case 'n':
                print = false;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                break;
        }
    }
    if (!serverMode && print) {
        fprintf(stderr, "%-9s\t%12s\n\n", "Server_ip", argv[argc - 1]);
    }

    if (serverMode) {
        pthread_create(&receiver_thread, NULL, receiver, NULL);
        pthread_join(receiver_thread, NULL);
    }
    else {
        pthread_create(&sender_thread, NULL, sender, "Hello\n");
        pthread_join(sender_thread, NULL);
    }

    if (signal(SIGINT, signal_hdnler) == SIG_ERR) {
        printf("Can't catch SIGINT\n");
    }
    while (1) {
        sleep(1);
    }
}
