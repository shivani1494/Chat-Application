#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <stdlib.h> /* for atoi() */
#include <stdbool.h>
#include <unistd.h> 
#define RCVBUFSIZE 90 /* Size of receive buffer */
#define MSGBUFSIZE 2000 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */


void LogOn();
void GetUserList();
void SendMessage();
void GetMessage();
void StartChat();
void ReceiveMessage();
void Chat();
char clientOption[RCVBUFSIZE]; /* Buffer for client option */
int recvMsgSize; /* Size of received message */

char messageFromAlice[MSGBUFSIZE];
char messageFromBob[MSGBUFSIZE];

char  *currentUser;

void GetUserList(int clntSocket) 
{
    char *userList = malloc(RCVBUFSIZE);
    userList = "The users are \n 1. Alice \n 2. Bob \n";

    if (send(clntSocket, userList, strlen(userList), 0) != strlen(userList))
        DieWithError("send() failed");
}

void LogOn(int clntSocket)
{

    char name[RCVBUFSIZE]; /* Buffer for name */
    char pass[RCVBUFSIZE]; /* Buffer for pass */
    int recvMsgSize; /* Size of received message */

    if ((recvMsgSize = recv(clntSocket, name, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    if ((recvMsgSize = recv(clntSocket, pass, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    name[recvMsgSize+1] = '\0'; 
    printf("%s has logged in!", name);
    printf("\n");

    if (name[0] == 'A') 
        currentUser = "Alice";
    else
        currentUser = "Bob";
    printf("Current User is %s \n", currentUser);
}

void ReceiveMessage(int sock)
{
    //maintain a data structure to store messages for Bob and Alice in LIFO data structure       
    if(currentUser == "Alice")
    {        
        if ((recvMsgSize = recv(sock, messageFromAlice, MSGBUFSIZE - 1, 0)) < 0)
            DieWithError("recv() failed ... here");
    }    
    else
    {        
        if ((recvMsgSize = recv(sock, messageFromBob, MSGBUFSIZE - 1, 0)) < 0)
            DieWithError("recv() failed ... here");
    }

}

void SendMessage(int clntSocket)
{
    //use the data structure and pop out the message and send based on the user.
    printf("Send Message");

    if(currentUser == "Bob")
    {
        //printf("here sending for bob");
        if (send(clntSocket, messageFromAlice, strlen(messageFromAlice), 0) != strlen(messageFromAlice))
            DieWithError("send() failed");

    }    
    else
    {
        if (send(clntSocket, messageFromBob, strlen(messageFromBob), 0) != strlen(messageFromBob))
            DieWithError("send() failed");

    }
    
}

void StartChat(int clntSocket)
{
    fflush(stdout);
    char *disconnectMsg = malloc(RCVBUFSIZE);
    disconnectMsg = "Disconnected from Server \n";

    if (send(clntSocket, disconnectMsg, strlen(disconnectMsg), 0) != strlen(disconnectMsg))
        DieWithError("send() failed");
    
    //printf("Here!");

    // /* See if there is more data to receive */
    // //why do we need a receive here?????
    // if ((recvMsgSize = recv(clntSocket, clientOption, RCVBUFSIZE, 0)) < 0)
    //     DieWithError("recv() failed");
    //sleep(15);
    //calling close socket here results in weird behavior, why so?
    //close(clntSocket); /* Close client socket */
    //what happens if you don't close the client socket?
    
}

void HandleTCPClient(int clntSocket)
{
    currentUser = malloc(RCVBUFSIZE);
    
    while (true)
    {
        //are we suppose to debug the template code as well??
        //this is buggy code! - when recv returns 0 then it is an indication that
        //peer has orderly closed the socket, however the way it is handled here is not accounting for that         
        // if ((recvMsgSize = recv(clntSocket, clientOption, RCVBUFSIZE, 0)) < 0)
        //     DieWithError("recv() failed");            
        //please dont give us buggy code _/\_

        recvMsgSize = recv(clntSocket, clientOption, RCVBUFSIZE, 0);

        if(recvMsgSize == 0)
            break;
        else if (recvMsgSize < 0)
            DieWithError("recv() failed");
                
        int i = atoi(clientOption);   
        printf("Client Option is %d \n", i);
        switch(i)
        {
            case 0: 
                LogOn(clntSocket);
                break;
            case 1:                 
                GetUserList(clntSocket);                
                break;
            case 2:                    
                ReceiveMessage(clntSocket);                
                break;
            case 3:                 
                SendMessage(clntSocket);                
                break;
            case 4:
                StartChat(clntSocket);
                break;
            case 5: 
                printf("LogOn();");
                //Chat();
                break;
            default:
                printf("Invalid Option");
        }        
        if(i == 4)
        {
            close(clntSocket);
            break;
        }
            
    }
}

/* Echo message back to client */
//if (send(clntSocket, clientOption, recvMsgSize, 0) != recvMsgSize)
    //DieWithError("send() failed");

// /* See if there is more data to receive */
// if ((recvMsgSize = recv(clntSocket, clientOption, RCVBUFSIZE, 0)) < 0)
//     DieWithError("recv() failed");


