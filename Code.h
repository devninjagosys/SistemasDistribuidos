/*
 *  Header for the 'valentao' code
 *  Author: Amanda
 *
 */

#include <string.h>
#define BUFLEN 2048
using namespace std;
// Global variables
int server_socket;         // This process' socket
// TODO: Should it be protected by lock?

char* messageBuffer;
int messageLength;
const char* delimiter;
vector<int> client_sockets;


// Message types
enum MessageTyp {
    m_eleicao = 1,
    m_ok = 2,
    m_lider = 3,
    m_vivo = 4,
    m_vivo_ok = 5
};



// Main funtions
int main(int argc, char* argv[]);

void interface();
void receiveMsgFromClients();
int setupServerSocket(int port);
void sendMsgToClient(int port);
void setupClientSocket(int port);
//void leader();


// Auxiliary funtions

//bool checkLeader();



class ProcessClient
{
    public:

    struct sockaddr_in myaddr, remaddr;

};