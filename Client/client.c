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
char *name;
char *password;
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
void ReceiveMessage(int clntSocket);
void SendMessageToServer();
void Receive(int sock, char *receivedData, bool print);
void InputAndSend(int sock);

//================= HELPER FUNCTIONS ==============//

void Receive(int sock, char *receivedData, bool print)
{
    int recvMsgSize; /* Size of received message */
    recvMsgSize = recv(sock, receivedData, RCVBUFSIZE, 0);
    if(recvMsgSize == 0)
    {
        printf("Socket Closed, disconnecting");
        return;
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
        close(sock); /* Close client socket */
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

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);
        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
            DieWithError("accept() failed");
        /* clntSock is connected to a client! */
        //printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        ReceiveMessage(clntSock);
    }
}

void ReceiveMessage(int sock)
{
    char *data = malloc(RCVBUFSIZE);
    int recvMsgSize; /* Size of received message */

    //receiving friend option - 5    
    Receive(sock, data, true);
        
    //Receiving friend name
    Receive(sock, data, true);
    printf("%s has logged in to chat. \n", data);
    
    //ignore password this is a small hack to use the same function below
    Receive(sock, data, true);

    while(true)
    {
        Receive(sock, data, true);
        printf("\n Please enter your message: (type BYE to exit) \n");  
        InputAndSend(sock);
    }
}

void ChatWithFriend()
{
    //close(sock);
    ifLoggedIn = false;
    unsigned short portN = 8000;
    ConnectAndLogOn(portN);
    
    //Exchange messages with friend
    char *receivedData = malloc(RCVBUFSIZE);
    while(true)
    {
        printf("\n Please enter your message: (type BYE to exit) \n");    
        InputAndSend(sock);
        Receive(sock, receivedData, true);        
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
    name = NULL;
    password = NULL;
    sock = NULL;

    /* Dynamic Allocation */
    name = malloc(SIZE);
    password = malloc(SIZE);
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
    //==============important=========uncomment this
    else
        printf("Connected!\n");
        //printf("Connected! \n");

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

