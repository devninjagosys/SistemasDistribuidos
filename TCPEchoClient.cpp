#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define SIZE_BUFFER 256

using namespace std;

int error(const char *msg)
{
    perror(msg);
    return EXIT_FAILURE;
}

int generateRandomNumber(int current_number)
{   
    if (current_number <=0 )
    {
      current_number = 1;
    }
    srand(time(NULL));
    int increasing = rand() % 100 + 1; 
    return (current_number += increasing) ;
};


int main(int argc, char *argv[])
{
    int socketFileDescript, portNumber ,statusWrite ,statusRead ,dataMsg ,numberProduction ,currentProduction = 0, numberMsgs = 0 ;
    struct sockaddr_in serverAddress;
    struct hostent *server;

    char charDataMsg[SIZE_BUFFER];
    if (argc < 3) {
       fprintf(stderr,"Usage %s Hostname Port NumberOfProductions \n", argv[0]);
       exit(0);
    }
    portNumber = atoi(argv[2]);
    numberProduction = atoi(argv[3]);
    socketFileDescript = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescript < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    //Line to understand
    bcopy((char *)server->h_addr,(char *)&serverAddress.sin_addr.s_addr,server->h_length);
    serverAddress.sin_port = htons(portNumber);
    if (connect(socketFileDescript, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) 
        error("ERROR connecting");
    while(numberProduction>0)
    {   
        //printf("Please enter the message: ");
        bzero(charDataMsg,SIZE_BUFFER);
        //fgets(charDataMsg,SIZE_BUFFER-1,stdin);
        currentProduction = generateRandomNumber(currentProduction);
        sprintf(charDataMsg, "%d", currentProduction);
        printf("Sending to the Consumer: %d\n",currentProduction);
        statusWrite = write(socketFileDescript, charDataMsg, strlen(charDataMsg));
        if (statusWrite < 0) 
            error("ERROR writing to socket");
        bzero(charDataMsg,256);
        statusRead = read(socketFileDescript, charDataMsg, SIZE_BUFFER);
        if (statusRead < 0)     
            error("ERROR reading from socket");
        printf("%s\n", charDataMsg);
        numberProduction-=1;
        numberMsgs+=1;
    }
    printf("Total number of messages exchanged: %d\n",numberMsgs);
    close(socketFileDescript);
    return 0;
}