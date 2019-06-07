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
int msgCount=0;


// Message types
enum MessageTyp {
    m_eleicao = 1,
    m_ok = 2,
    m_lider = 3,
    m_vivo = 4,
    m_vivo_ok = 5
};


class ProcessClient
{
    int actualLider;
    pid_t myPid;
    std::string processName;
    public:
        struct sockaddr_in myaddr,remaddr;
        int client_socket_ID;
        int myport;
        ProcessClient(int lider,std::string name,int port)
        {
            myPid = getpid();
            actualLider = lider;
            processName = name;
            myport = port;
        }
        void setLider(int lider)
        {
            actualLider=lider;
        }
        int getLider()
        {
            return actualLider;
        }
        int getPid()
        {
            return (int)myPid;
        }

};

// Main funtions
int main(int argc, char* argv[]);
// AUX functions
void interface();
void receiveMsgFromClients(int flag);
int setupServerSocket(int port);
int setupClientSocket(ProcessClient& clientProcess);
void sendMsgToClient(int flag,ProcessClient& clientProcess);
