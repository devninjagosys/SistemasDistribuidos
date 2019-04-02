/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#define PORT 8080
#define SIZE_BUFFER 256

#define LOG_ON 1

using namespace std;

int error(const char *msg)
{
    perror(msg);
    return EXIT_FAILURE;
}

int isPrime(int number)
{   for(int i = 2; i <= number/2; ++i)
    {
        if(number % i == 0)
        {
          return 0;
        }
    }
    return 1;
};

int main(int argc, char *argv[])
{
  int socketFileDescript, newsocketFileDescript, portNumber, dataMsg , isPrimeProduction ,numberOfMessages = 0;
  socklen_t clientLength;
  char charDataMsg[SIZE_BUFFER];
  struct sockaddr_in serverAddress, clientAddress;
  int statusSocket;
  if (argc < 2)
  {
    fprintf(stderr,"WARNING: Port number not given. Set to default.\n");
    portNumber = PORT;
  } else {
    portNumber = atoi(argv[1]);
  }

  // Creating a New Socket
  // Socket File descriptor = socket(int domain, int type, int protocol)
  socketFileDescript =  socket(AF_INET, SOCK_STREAM, 0);
  if (socketFileDescript < 0) 
  {
    error("ERROR: Opening Socket file Descriptor failed");
  }

  //Clear Address Structure
  bzero((char *) &serverAddress, sizeof(serverAddress));

  // Setup the host_addr structure for use in bind call
  serverAddress.sin_family = AF_INET;

  // Automatically be filled with current host's IP address
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Convert short integer value for port must be converted into network byte order
  serverAddress.sin_port = htons(portNumber);

  // Bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
  // bind() passes file descriptor, the address structure,and the length of the address structure
  // This bind() call will bind the socket to the current IP address on port, portNumber
  if (bind(socketFileDescript, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) 
      error("ERROR: Binding failure");
  printf("The Server is UP - LOOK TO THE POWER !\n");

  // This listen() call tells the socket to listen to the incoming connections.
  // The listen() function places all incoming connection into a backlog queue until accept() call accepts the connection.
  // Here, we set the maximum size for the backlog queue to 5.
  listen(socketFileDescript,5);

  // The accept() call actually accepts an incoming connection
  clientLength = sizeof(clientAddress);

  /* This accept() function will write the connecting client's address info into 
    the address structure and the size of that structure is clientLength.
    The accept() returns a new socket file descriptor for the accepted connection.
    So, the original socket file descriptor can continue to be used for accepting new connections 
    while the new socker file descriptor is used for
    communicating with the connected client.*/
  newsocketFileDescript = accept(socketFileDescript,(struct sockaddr *) &clientAddress, &clientLength);
  if (newsocketFileDescript < 0) 
          error("ERROR: Accept failure");
  printf("server: got connection from %s port %d\n",inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));


  while(true)
  {
    statusSocket = read(newsocketFileDescript,charDataMsg,SIZE_BUFFER);
    if (statusSocket < 0)
    {
      error("ERROR: Sending Message to Socket file Descriptor Failed");
      break;
    }
    dataMsg =  atoi(charDataMsg);
    if (dataMsg == 0)
    { 
      // cout << "The Producer received an Unexpected message:" << charDataMsg <<endl;
      cout << "The consumer received a zero (0). Will terminate process." << endl;
      break;
    }
    cout << "The Consumer Received from the Producer: " << dataMsg << endl;
    isPrimeProduction = isPrime(dataMsg);
    //sprintf (charDataMsg, "%d", isPrimeProduction);


    if (isPrimeProduction == 1 )
    {
      if (LOG_ON) {
        cout << "Consumer is sending to the Producer: Received a Prime Number " << endl;
      }
        sprintf (charDataMsg, "Consumer : Received a Prime Number (%d)", dataMsg);
    }
    else
    {
      if (LOG_ON) {
        cout << "Consumer is sending to the Producer: Received a Non-Prime Number " << endl;
      }
      sprintf (charDataMsg, "Consumer : Received a Non-Prime Number (%d)", dataMsg);
    }

    // This send() function sends the 13 bytes of the string to the new socket
    statusSocket = send(newsocketFileDescript,charDataMsg, SIZE_BUFFER, 0);

    if (statusSocket < 0)
    {
      error("ERROR: Sending Message to Socket file Descriptor Failed");
    }
    bzero(charDataMsg,SIZE_BUFFER);
    numberOfMessages+=1;
  }
  printf("The Communication has Finished!\n");
  printf("Total number of messages exchanged: %d\n",numberOfMessages*2);
  close(newsocketFileDescript);
  close(socketFileDescript);
  return 0; 
}
