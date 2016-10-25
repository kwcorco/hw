#include "starLord.h"
#define NUM_MSGS 64

int main(int argc, char *argv[]) {

	// Ensure that user ran with correct syntax and # of arguments. If not, exit
	if (argc < 9) {
		printf("Syntax: ./ahsoka -s <server addr> -p <port> -h <hostHeader> ");
		printf("-a1 <message1>...-aN <messageN>\n");
		exit(1);
	}

	// Declare variables
	char *servIP;					// server IP address 
	char *servPort;				// server port number
	char *hostHeader; 			// host header
	char *msgsArgs[NUM_MSGS];	// array of messages parsed
	char *msgsToSend[NUM_MSGS]; // array of messages to send
	char *msgsSent;
	char *msgsReceived; // array of messages received
	//char *msgsExpected[NUM_MSGS]; // array of messaged expected to receive
	int msgsParsed = 0;			// number of msgs parsed so far from cmnd line
	bool validMsgFlag = true;
	bool validMsgLen = true;
	size_t messageLen;
	ssize_t numBytes;

	// initialize messages array with NUM_MSGS strings of 80 chars each
	int i;
	for (i=0; i<NUM_MSGS; i++) 
		msgsArgs[i] = (char *)malloc(80 * sizeof(char));

	// Parse command line arguments with flags and initialize variables
	int j=1;
	while (j < argc) {
		switch (argv[j][1]) {
			case 's':
				servIP = argv[j+1];
				break;
			case 'p':
				servPort = argv[j+1];
				break;
			case 'h':
				hostHeader = argv[j+1];
				break;
			case 'a':
				if (argv[j][3]-48 >= 0 && argv[j][3]-48 <= 9) {
					if (((int)(argv[j][2])-48) != ((msgsParsed+1)/10) ||
						((int)(argv[j][3])-48) != ((msgsParsed+1)%10)) {
							validMsgFlag = false;
					}
				}
				else if (((int)(argv[j][2])-48 != (msgsParsed+1))) {
					validMsgFlag = false;
				}
				if (!validMsgFlag) {
					printf("\nERROR with message flag order!\n");
					printf("messages must be sent on command line corresponding to");
					printf(" -aX flags,\nwhere X is an integer. the messages must");
					printf(" be sent in order that they wish\nto be sent; thus ");
					printf("the -aX flags must occur in ascending order, starting ");
					printf("with -a1.\nTry again.\n\n");
					exit(1);
				}
				else {
					if (strlen(argv[j+1]) > 80) {
						validMsgLen = false;
					}
				}
				if (!validMsgLen) {
					printf("\nERROR with mesage length!\n");
					printf("max length of message is 80 chars. Try again\n\n");
					exit(1);
				}
				else {
					msgsArgs[msgsParsed++] = argv[j+1];
					break;
				}
			default:
				if ((char)argv[j][0] != '-') {
					printf("Syntax: ./ahsoka -s <server addr> -p <port> ");
					printf("-h <hostHeader> -a <message1>...-aN <messageN>\n");
				}
				else {
					printf("only acceptable flags are -s, -p, -h, and -aX, ");
					printf("where X is an integer greater than 1.\n");
				}
				exit(1);
		}
		j+=2;
	}

	// Tell the system what kind of address info we want
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;
	addrCriteria.ai_socktype = SOCK_STREAM;
	addrCriteria.ai_protocol = IPPROTO_TCP;

	// Get address
	struct addrinfo *servAddr;
	int rtnVal = getaddrinfo(servIP, servPort, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		printf("getaddrinfo() failed: %s\n", gai_strerror(rtnVal));
		exit(1);
	}

	// Create a reliable stream socket using TCP
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
							servAddr->ai_protocol);
	if (sock < 0) {
		perror("socket() failed");
		exit(1);
	}
/*
	// Establish the connection to the server
	if (connect(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		perror("connect() failed");
		exit(1);
	}
*/
	// Declare new variables for sending/receiving and clock time
	_Bool send_success = false, recv_success = false;
	int attempts = 0;
	clock_t begin,end;
	begin = clock();

	
	int k;
	for (k=0; k<msgsParsed; k++) {
		connect(sock, servAddr->ai_addr, servAddr->ai_addrlen);
		msgsToSend[k] = malloc(strlen(msgsArgs[k])
							+strlen("GET /add? HTTP/1.1\nHost: ")
							+strlen(hostHeader));
		memset(msgsToSend[k], 0, strlen(msgsToSend[k]));
		strcat(msgsToSend[k], "GET /add?");
		strcat(msgsToSend[k], msgsArgs[k]);
		strcat(msgsToSend[k], " HTTP/1.1\nHost: ");
		strcat(msgsToSend[k], hostHeader);
	
		// Send the string to the server
		messageLen = strlen(msgsToSend[k]);
		numBytes = send(sock, msgsToSend[k], messageLen, 0);
		printf("msg sent: %s\n", msgsToSend[k]);	
	}

/*
	// Receive final re-inverted string back from caseInverter
	char final[MAXSTRINGLENGTH + 1];
	bzero(final, sizeof(final));
	recv_success = false;
	while (!recv_success) {
		numBytes = recv(sock, final, BUFSIZE - 1, 0);
		if (numBytes == invertedLen) 
			recv_success = true;
		else if (numBytes != invertedLen) {
			printf("recv() error: received unexpected number of bytes;\n");
			printf("attempting to receive again...\n");
		}
		else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr*)&fromAddr))
			printf("recv() error: received a packet from unknown source\n");
		else
			printf("recvfrom() failed; attempting to receive again...\n");
	}

	// Null-terminate inverted and final message
	inverted[messageLen] = '\0';
	final[messageLen] = '\0';

	// Verify that initial msg and final doubly-inverted msg are identical
	_Bool verified = !strcmp(message, final);

	// End clock and print communication stats
	end = clock();
	double time_spent = ((double)(end - begin)) / CLOCKS_PER_SEC;
	printf(" %d	%.6f	%s	%s	%s\n", attempts, time_spent, message, inverted,
			 verified ? "Verified" : "Not Verified");
	
	// Close socket and free addrinfo allocated in getaddrinfo()
	close(sock);
	freeaddrinfo(servAddr);
*/
	return 0;
}

