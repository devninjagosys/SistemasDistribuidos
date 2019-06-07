#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

#include "Code.h"


using namespace std;


// User interface management
void interface(){
    /*
     *  Thread responsible for the user interface
     *
     */

    // Setup to read keys
    struct termios oldSettings, newSettings;

    tcgetattr( fileno( stdin ), &oldSettings );
    newSettings = oldSettings;
    newSettings.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr( fileno( stdin ), TCSANOW, &newSettings );

    while(1){
        // Read letter
        char l;
        char* l_p = &l;

        fd_set set;
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO( &set );
        FD_SET( fileno( stdin ), &set );
        int res = select( fileno( stdin )+1, &set, NULL, NULL, &tv );
        if ( res > 0 ){
            read( fileno( stdin ), l_p, 1 );
            // cout << "Pressed key: " << l << endl;
            l = tolower(l);
        } else if ( res < 0 ){
            perror( "select error" );
            break;
        }

        // Check current leader
        if ( strncmp(l_p, "l", 1) == 0 ){
            cout << "Pressed L" << endl;
            cout << "\t Checking on leader" << endl;
            // checkLeader();
            // TODO: Initiate leader check process.
            // Might be better to take leader check thread out of sleep...
        }

        // Emulate process failure
        else if ( strncmp(l_p, "f", 1) == 0 ){
            cout << "Pressed F" << endl;
            cout << "\t Process will emulate failure" << endl;
            // TODO: Make process non-responsive to messages
        }

        // Recuperate process from (fake) failure
        else if ( strncmp(l_p, "r", 1) == 0 ){
            cout << "Pressed R" << endl;
            cout << "\t Process will start answering again" << endl;
            // TODO: Make process non-responsive to messages
        }

        // Print Statistics
        else if ( strncmp(l_p, "s", 1) == 0 ){
            cout << "Pressed S" << endl;
            cout << "\t Printing statistcs" << endl;
            // TODO: Make process non-responsive to messages
        }
    }
}





int main(int argc, char* argv[]){
    // TODO: Function arguments
    int myPort = 8080;
    messageLength = 1024;
    messageBuffer = new char[messageLength];
    delimiter = "|";
    // ----
    if(argc < 2)
    { 
        printf("Enter a valid integer."); 
        return 0;
    }
    int numTimes = atoi(argv[1]);
    //std::cout<<numTimes<<endl;
    if (numTimes==1)
    {
    if ( setupServerSocket(myPort) == -1 ){
        cout << "ERROR: Could not setup socket" << endl;
        exit(1);
    }
    receiveMsgFromClients(0);
    }
    else
    {
    setupClientSocket(8080);
    sendMsgToClient(8080,0);
    }
    // interface();
}






// Auxiliary Functions ----------------------------------------------------------

int setupServerSocket(int port)
{
    /*
     *  Thread responsible for create a server socket
     *  
     */
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if ( bind( server_socket, (struct sockaddr *)&address, sizeof(address) ) < 0 ){
        cout << "ERROR: Unable to bind socket" << endl;
        perror("bind failed");
        return -1;
    }
    return 0;
}

void receiveMsgFromClients(int flag)
{
    /*
     *  Thread responsible for receiving messages from other processes and
     *  Flag 1 -> Send ack for confirmation
     */
    struct sockaddr_in client_address;
	socklen_t addrlen = sizeof(client_address);
    int count=0;
	while (true) 
    {
		char buffer[500];

		// read content into buffer from an incoming client
		int recvlen = recvfrom(server_socket, buffer, sizeof(buffer), 0,
		                   (struct sockaddr *)&client_address,
		                   &addrlen);

		// inet_ntoa prints user friendly representation of the
		// ip address
        if (recvlen>0)
        {
            buffer[recvlen] = 0;
            printf("received: '%s' from client %s , Count:%d\n", buffer,
		       inet_ntoa(client_address.sin_addr),count);
        }
		// send same content back to the client ("echo")
        if (flag==1)
        {
        printf("sending response \"%s\"\n", buffer);
		sendto(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address,addrlen);
        }
        count++;


    }
}

int setupClientSocket(int port)
{
    /*
     *  Thread responsible for create a client socket
     *  
     */
    struct sockaddr_in myaddr, remaddr;
	int client_socket;
	int recvlen;		/* # bytes in acknowledgement message */
	char server[] = "127.0.0.1";	/* change this to use a different server */

	if ((client_socket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    {
		printf("socket created\n");
    }
    memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

    if (bind(client_socket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) 
    {
		perror("bind failed");
		exit(0);
    }
    client_sockets.push_back(client_socket);
    return 1;
}
void sendMsgToClient(int port,int flag)
{
    /*
     *  Thread responsible for sending msg in a client socket
     *  
     */
    struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);
    memset((char *) &remaddr, 0, sizeof(remaddr));
    char client_buffer[BUFLEN];	/* message buffer */
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(port);
    char server[] = "127.0.0.1";
    int recvlen;
    int count=0;
    int temp_client_socket = client_sockets.back();
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	printf("Sending packet %d to %s port %d\n", count, server, port);
	sprintf(client_buffer, "This is packet %d", count);
	if (sendto(temp_client_socket, client_buffer, strlen(client_buffer), 0, (struct sockaddr *)&remaddr, addrlen)==-1) {
		perror("sendto");
		exit(1);
	}
	/* now receive an acknowledgement from the server */
    if(flag==1)
    {
	recvlen = recvfrom(temp_client_socket, client_buffer, BUFLEN, 0, (struct sockaddr *)&remaddr, &addrlen);            
    if (recvlen >= 0) 
        {
        client_buffer[recvlen] = 0;	/* expect a printable string - terminate it */
        printf("received message: \"%s\"\n", client_buffer);
        }
    }
}
