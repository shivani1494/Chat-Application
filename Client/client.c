//multi threading, emulate an actual chatting scenario?
#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <stdbool.h> 
#include <string.h>

#define RCVBUFSIZE 1000 /* Size of receive buffer */
#define MAXPENDING 5 /* Maximum outstanding connection requests */
int SIZE = 1000; /* const */

//global variables necessary for program execution
int sock; /* Socket descriptor */
struct sockaddr_in serverAddr; /* Server address */

char receivedData[RCVBUFSIZE]; /* Buffer for echo string */
int bytesRcvd, totalBytesRcvd; /* Bytes read in single recv() and total bytes read */

char *portNum; /* Echo server port */
char *serverIP; /* Server IP address (dotted quad) */
bool ifLoggedIn;
char *clientOption;

int servSock; /* Socket descriptor for server */
int clntSock; /* Socket descriptor for client */
struct sockaddr_in echoServAddr; /* Local address */
struct sockaddr_in echoClntAddr; /* Client address */
unsigned short echoServPort; /* Server port */
unsigned int clntLen; /* Length of client address data structure */

bool CheckClientLoggedIn();
void ConnectAndLogOn(unsigned short portNumint);
void DieWithError(char *errorMessage);
void GetUserList();
void ReceiveDataFromServer();
void PrintUserOptionsAndRedirectRequests();
void LogOn();
void StartChat();
void ChatWithFriend();
bool ReceiveMessage(int clntSocket);
void SendMessageToServer();
bool Receive(int sock, char *receivedData, bool print);
void InputAndSend(int sock);

//================= HELPER FUNCTIONS ==============//

bool Receive(int sock, char *receivedData, bool print)
{
    int recvMsgSize; /* Size of received message */
    recvMsgSize = recv(sock, receivedData, RCVBUFSIZE, 0);
    if(recvMsgSize == 0)
    {
        printf("Socket Closed, disconnecting from your friend\n"); 
        return true;
    }
    else if (recvMsgSize < 0)
        DieWithError("recv() failed");
    
    if(print) 
    {
        receivedData[recvMsgSize] = '\0'; 
        printf(receivedData);
        //printf("whats up why- exiting \n");
    }
    //handle bye properly!
    if(receivedData[0] == 'b' && receivedData[1] == 'y' && receivedData[2] == 'e')
    {
        close(sock); /* Close client socket */
        printf("Socket Closed, disconnecting from your friend\n"); 
        return true;
    }
    return false;
        
}

void InputAndSend(int sock)
{
    char line[200000];
    //fflush(stdin);
    //scanf("%s", line);
    //scanf("%[^\n]", line);
    //sleep(20);
    fgets(line, 20000, stdin);
    
    if (send(sock, line, strlen(line), 0) != strlen(line))
        DieWithError("send() sent a different number of bytes than expected");
}

bool CheckClientLoggedIn()
{    
    if(ifLoggedIn)
        return true;
    return false;
}

void LogOn()
{

    if (send(sock, clientOption, strlen(clientOption), 0) != strlen(clientOption))
        DieWithError("send() sent a different number of bytes than expected");

    //Get log in credentials
    //fflush(stdin);
    //fflush(stdout);
    printf("Please enter your username- ");
    InputAndSend(sock);
    
    printf("Please enter your password- ");
    InputAndSend(sock);

    //receive success from server
}

//================= HELPER FUNCTIONS ==============//

void PrintUserOptionsAndRedirectRequests() 
{
     
    printf("0. Connect to the server \n");
    printf("1. Get the user list \n");
    printf("2. Send a message \n");
    printf("3. Get my messages \n");
    printf("4. Initiate a chat with my friend \n");
    printf("5. Chat with my friend \n");

    char option[10];
    printf("Please select from one of the above options-" );
    scanf("%s", option);
    //we need to do this to remove the newline character
    //if the newline character exists in the buffer then the next subsequest read
    //will be skipped because of the newline in the buffer already which will be read first
    //I wonder then what on earth does flush do huh!
    getchar();
    int optionInt = atoi(option);
    clientOption = option;
    unsigned short portN = 7000;
    
    switch(optionInt)
    {
        case 0:
            ConnectAndLogOn(portN);
            break;
        case 1:             
            ReceiveDataFromServer();
            break;
        case 2:             
            SendMessageToServer();
            break;
        case 3:             
            ReceiveDataFromServer();
            break;
        case 4:            
            StartChat();
            break;
        case 5: 
            ChatWithFriend();
            break;
        default:
            printf("Invalid Option");
            break;
    }
}

void StartChat()
{
    if (send(sock, clientOption, strlen(clientOption), 0) != strlen(clientOption))
        DieWithError("send() sent a different number of bytes than expected");

    char *data = malloc(RCVBUFSIZE);
    Receive(sock, data, true);
    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError( "socket () failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
    echoServAddr.sin_family = AF_INET; /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(8000); /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError ("bind () failed");
    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
    bool socketClosed = false;
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);
        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
            DieWithError("accept() failed");
        /* clntSock is connected to a client! */
        //printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        socketClosed = ReceiveMessage(clntSock);
        if (socketClosed)
            return;
    }
}

bool ReceiveMessage(int sock)
{
    char *data = malloc(RCVBUFSIZE);
    int recvMsgSize; /* Size of received message */
    bool socketClosed = false;
    //receiving friend option - 5    
    Receive(sock, data, false);
        
    //Receiving friend name
    Receive(sock, data, false);
    printf("%s has logged in to chat. \n", data);
    
    //ignore password this is a small hack to use the same function below
    Receive(sock, data, false);

    while(true)
    {
        socketClosed = Receive(sock, data, true);
        if(socketClosed)
            return true;
        printf("\nPlease enter your message: (type BYE to exit): ");  
        InputAndSend(sock);
    }
    
    return false;
}

void ChatWithFriend()
{
    //close(sock);
    ifLoggedIn = false;
    unsigned short portN = 8000;
    ConnectAndLogOn(portN);
    bool socketClosed = false;
    
    //Exchange messages with friend
    char *data = malloc(RCVBUFSIZE);
    while(true)
    {
        printf("\n Please enter your message: (type BYE to exit): ");    
        InputAndSend(sock);
        socketClosed = Receive(sock, data, true);
        if (socketClosed)
            return;        
    }
}

void SendMessageToServer()
{
    if (send(sock, clientOption, strlen(clientOption), 0) != strlen(clientOption))
        DieWithError("send() sent a different number of bytes than expected");

    printf("Type your message \n");
    InputAndSend(sock);
    
    //technically should receive message from server that message was received!
    printf("Message Received! ");
}

void ReceiveDataFromServer()
{
    char *data = malloc(RCVBUFSIZE);
    bool ifLoggedIn = CheckClientLoggedIn();
    if (!ifLoggedIn)
    {
        printf("Youre not logged in! - Please log on");
        return;
    }

    if (send(sock, clientOption, strlen(clientOption), 0) != strlen(clientOption))
        DieWithError("send() sent a different number of bytes than expected");

    Receive(sock, data, true);
}

void ConnectAndLogOn(unsigned short portNumint) 
{
       
    ifLoggedIn = CheckClientLoggedIn();
    if (ifLoggedIn)
    {
        printf("youre logged in already!");
        return;
    }
    ifLoggedIn = true;
    portNum = NULL;
    serverIP = NULL;
    sock = NULL;

    /* Dynamic Allocation */
    serverIP = malloc(SIZE);
    portNum = malloc(SIZE);
    sock = malloc(SIZE);

    printf("Please enter the server IP: ");
    scanf("%s", serverIP);
    getchar();
    //fflush(stdin);
    //fflush(stdout);
    
    printf("Please enter the port number: ");
    scanf("%s", portNum);
    getchar();
    // fflush(stdin);
    // fflush(stdout);
    
    //hard coding to avoid typing every time!
    serverIP = "127.0.0.1";
    // portNumint = 9000;

    //why can't we print sock??
    //getting seg fault
    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket () failed");

    //printf("Socket created!\n");

    /* Construct the server address structure */
    memset(&serverAddr, 0, sizeof(serverAddr)); /* Zero out structure */
    serverAddr.sin_family = AF_INET; /* Internet address family */
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); /* Server IP address */
    serverAddr.sin_port = htons(portNumint); /* Server port */

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        DieWithError("connect () failed");
    else
        printf("Connected!\n");\

    // if(close(sock) < 0)
    //     DieWithError("Connection to server was not closed properly");
    
    //fflush(stdout);
    LogOn();
}

int main(int argc, char *argv[])
{
    ifLoggedIn = false;
    while(true)
    {
        PrintUserOptionsAndRedirectRequests();
        printf("\n");
    }
    printf("\n"); /* Print a final linefeed */
    close(sock);
    exit(0);
}

//socket interface
//send and receive
//how is the socket created?
//how is that enabling a connect between the sender and receiver
//note that the sends and the receives have to be timed
//such that the server is actually waiting to receive and the 
//client has not sent its message yet
//if the client has already sent its message and the control has not
//reached to the receive call yet then the server will infinitely wait
//to receive the message however the message has already been sent from the server
//how is this actually handled in practise because there can be multiple
//such situations in which the client is faster or vice versa in that case 
//like the server could be disconnecting from the previous client and 
//is can be used now because the socket from previous client is closed
//but the server is not ready yet to receive incoming requests
//while the second client has already sent some!

//when socket is closed-- is a message sent to the client/server waiting on it
//because when the socket is closed there is no send
//but some message must be received to detect that the receive == 0, how is this actually happening?

