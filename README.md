# Assignment1 - TCPEchoClient
-The client will contact the server and request a list of text files that server has in its
possession. The server will have 3 different text files: song.txt, poem.txt, quote.txt. Server
will send the list of files to the client. Client will then print the selection for the user and
ask the user to select a file that user would like to receive. User will make a selection and
client will send it to the server. Server will then send to the client the contents of the
requested file. All original formatting should be preserved. Client will print the received
text to the standard output and close the connection.

# Assignment2 - UDPPing
-Develop a program called udping that will act as either a udp ping server (which simply echoes udp ping
packets back to the source), or a udp ping client which works similarly to the standard ping program.
The program must support the following command line options provided in any order (default values after parentheses):

-c ping-packet-count (stop after sending this many) 0x7fffffff

-i ping-interval (interval in seconds between ping sends) 1.0

-p port number (the port number the server is using) 33333

-s size in bytes (of the application data sent) 12

-n no_print (only print summary stats) print all

-S Server (operate in server mode) client mode
