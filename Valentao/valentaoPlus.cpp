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
#include <sstream>
#include <algorithm>
#include <pthread.h>
#include <chrono>
#include <mutex>  
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "valentao.h"


using namespace std;

// User interface management
void* interface(void*){
    /*
     *  Thread responsible for the user interface
     *
     */
    cout << "In interface thread" << endl;
    cout<< "Press [(L):Cheking on Leader] - [(F):Emulate failure] - [(R):Recover from failure] - [(S):Print Statistics] - [(0)-Stop printing]"<<endl;
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
            cout << "Pressed key: " << l << endl;
            l = tolower(l);
        } else if ( res < 0 ){
            perror( "select error" );
            break;
        }
        //cout<<"Get from terminal:"<<l_p<<endl;
        // Check current leader
        if ( strncmp(l_p, "l", 1) == 0 ){
            cout << "Pressed L: Checking on leader" << endl;

            if ((get_leaderIdx()) == -1) {
                // TODO: lock in cout!!
                cout << "\t Am already leader. Can't check on myself." << endl;
            }
            else if (get_electionSize()!= 0){
                cout << "\t Already in leader election. No leader to check on." << endl;
            } else if (get_isCheckingOnLeader()) {
                cout << "\t Already checking on leader. Will wait for the results." << endl;
            } else if (!(get_isOperational())) {
                cout << "\t Am emulating failure. Can't send messages "
                     << "unless you press recovery button (R)." << endl;
            } else {
                set_isCheckingOnLeader(true);

                char buffer[messageLength];
                sprintf(buffer, "%i%s%i%s", m_vivo, delimiter,
                                            selfID, delimiter );

                cout << "Messaging process. (" << buffer << ")" << endl;

                set_leaderAnswered(false);
                processes[get_leaderIdx()].sendMessage(buffer);
                //increasing_msgCount(m_vivo);
                
                increment_outMsgCount();

                sleep(2); // Wait to see if there'll be an answer

                set_isCheckingOnLeader(false);
                set_outElection(false);
            }
        }

        // Emulate process failure
        else if ( strncmp(l_p, "f", 1) == 0 ){
            // TODO: locks! (both on flag and cout)
            cout << "Pressed F: Process will emulate failure" << endl;
            set_isOperational(false);

        }

        // Recuperate process from (fake) failure
        else if ( strncmp(l_p, "r", 1) == 0 ){
            // TODO: locks! (both on flag and cout)
            cout << "Pressed R: Process will start answering again" << endl;
            set_isOperational(true);
        }

        // Print Statistics
        else if ( strncmp(l_p, "s", 1) == 0 ){
            cout << "Pressed S: Printing statistcs" << endl;
            // TODO: Make process non-responsive to messages
            int temp_electionNumber = get_electionNumber();
            cout << "---------------------------" << endl;
            if ((get_leaderIdx()) == -1) {
                cout << "| leader          |  " << selfID << " | -" <<"Election Number("<<temp_electionNumber<<")"<<endl;
            } else {
                cout << "| leader          |  " << processes[get_leaderIdx()].myport << " | -" <<"Election Number("<<temp_electionNumber<<")"<<endl;
            }
            int temp_totalElectionMsgs = get_totalElectionMsgs();
            int temp_inMsgCount = get_inMsgCount();
            int temp_outMsgCount = get_outMsgCount();
            mtx_msgCount.lock();
            cout << "---------------------------" << endl;
            cout << " Message counting  "<<endl;
            cout << "     - ELEICAO -> " << msgCount[m_eleicao] << endl;
            cout << "     - OK -> " << msgCount[m_ok] << endl;
            cout << "     - LIDER -> " << msgCount[m_lider] << endl;
            cout << "     - VIVO -> " << msgCount[m_vivo] << endl;
            cout << "     - VIVO_OK -> " << msgCount[m_vivo_ok] << endl;
            cout<<"Total Msgs Election: "<<temp_totalElectionMsgs<<endl;
            cout<<"Total Msgs Received: "<<temp_inMsgCount<<endl;
            cout<<"Total Msgs Sent: "<<temp_outMsgCount<<endl;
            mtx_msgCount.unlock();
        }
    }
}


void* communication(void*){
    /*
     *  Thread responsible for receiving messages from other processes and
     *
     */
    cout << "In communication thread" << endl;

    struct sockaddr_in client_address;
    socklen_t addrlen = sizeof(client_address);

    while (true){
        // cout<<"Total Msgs Received: "<<inMsgCount<<endl;
        // cout<<"Total Msgs Sent: "<<(get_outMsgCount())<<endl;
        int temp_totalElectionMsgs;
        mtx_msgCount.lock();
        temp_totalElectionMsgs = msgCount[m_eleicao]+msgCount[m_ok]+msgCount[m_lider];
        mtx_msgCount.unlock();
        set_totalElectionMsgs(temp_totalElectionMsgs);

        char buffer[messageLength];
        // read content into buffer from an incoming client
        int recvlen = recvfrom(server_socket, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&client_address,
                             &addrlen);
        // inet_ntoa prints user friendly representation of the
        // ip address
        if (recvlen>0){
            buffer[recvlen] = 0;
            increment_inMsgCount();
            printf("received: '%s' from client %s , Count:%d\n", buffer,
                   inet_ntoa(client_address.sin_addr), get_inMsgCount());
        }
        

        // Interpreting message
        char* msgPart;
        int msgT;
        int msgSender;

        try {
            msgPart = strtok(buffer, delimiter);
            msgT = atoi(msgPart);   // Message type number

            // TODO: Put lock!!!
            //msgCount[msgT]++;
            increasing_msgCount(msgT);



            msgPart = strtok(NULL, delimiter);
            msgSender = atoi(msgPart);
        } catch(...) {
            cout << "ERROR: Message reading error" << endl;
            continue;
        }

        if (msgT == m_eleicao){

            msgPart = strtok(NULL, delimiter);
            int elecValueReceived = atoi(msgPart);
            if (get_isOperational()){
                //Stop any leader check nor operations
                set_isOperational(false);
                // If ID is bigger than the received, should initiate election
                if (selfElecValue > elecValueReceived){
                    // TODO: Sleep shortly to avoid sending too many elections

                    // Checking if has sent this election message already
                    bool messageSent = false;
                    electionVectorMutex.lock();
                    for (int i; i<ongoingElections.size(); i++){
                        if (ongoingElections[i] == msgSender){
                            messageSent = true;
                            break;
                        }
                    }
                    electionVectorMutex.unlock();
                    if (!messageSent){ // Hasn't sent messages for this election
                        char outMsg[messageLength];
                        sprintf( outMsg, "%i%s%i%s%i%s",
                                 m_ok, delimiter,
                                 msgSender, delimiter,
                                 selfElecValue, delimiter );

                        // Broadcast of m_ok message 
                        for (int i=0; i<N_PROC-1; i++){
                           if (processes[i].myport == msgSender){
                                cout<<"Sending Msg to:"<<msgSender<<endl;
                                processes[i].sendMessage(outMsg);
                                //increasing_msgCount(m_ok);
                                
                            }
                        }
                        increment_outMsgCount();
                        //electionVectorMutex.lock();
                        //ongoingElections.push_back(msgSender);
                        //electionVectorMutex.unlock();
                        //Creating auxiliar thread
                        //sleep(5);
                        erase_ongoingElections();
                        mtx_maior_ID.lock();
                        maior_ID = false;
                        mtx_maior_ID.unlock();
                        set_isOperational(true);
                        set_isCheckingOnLeader(false);
                        set_outElection(false);
                        set_leaderAnswered(false);
                        // pthread_t threadsAux;
                        // pthread_attr_t attrAux;
                        // pthread_attr_init(&attrAux);
                        // pthread_attr_setdetachstate(&attrAux, PTHREAD_CREATE_DETACHED);
                        // pthread_create(&threadsAux, &attrAux, leader, NULL );
                    }
                }
                maior_ID = true;
                set_isOperational(true);
            }
            // If ID is smaller than received it makes no sense to do anything else
        } else if (msgT == m_ok){
            // TODO: Check if this erasing should't be done after some thread notices the election is over or something
            set_isSilenced(true);
            electionVectorMutex.lock();
            for (int i = 0; i < ongoingElections.size(); i++){
              if (ongoingElections[i] == msgSender){
                  // Take out of elections
                  ongoingElections.erase( ongoingElections.begin()+i );
                  break;
              }
            }
            electionVectorMutex.unlock();

        } else if (msgT == m_lider){
            // Update leader ID
            set_outElection(true);
            set_isSilenced(false);
            for (int i=0; i<N_PROC-1; i++){
                if (processes[i].myport == msgSender){
                    set_leaderIdx(i);
                    break;
                }
            }
            maior_ID=false;
            erase_ongoingElections();
            set_electionNumber();
            set_alreadyOnThreadleader(false);


        } else if (msgT == m_vivo){
            // In this case I am the leader myself, so I should answer the inquiry
            if (get_isOperational()){
                if ((get_leaderIdx()) == -1){ // NOTE: If I am the leader, leaderIdx is -1
                    char outMsg[messageLength];
                    sprintf( outMsg, "%i%s%i%s",
                             m_vivo_ok, delimiter, selfID, delimiter );

                    // Sending VIVO_OK message back
                    for (int i=0; i<N_PROC-1; i++){
                        if (processes[i].myport == msgSender){ // NOTE: Change if I change selfID configuration!!
                            processes[i].sendMessage(outMsg);
                            //increasing_msgCount(m_vivo_ok);
                            increment_outMsgCount();
                            break;
                        }
                    }
                } else {
                    cout << "ERROR: Received leader check message when I am not the leader" << endl;
                }
            }
            // TODO: Check this

        }   else if (msgT == m_vivo_ok){
            mtx_vivo_ok.lock();
            number_vivo_ok++;
            mtx_vivo_ok.unlock();
            set_outElection(false);
            if (msgSender == processes[get_leaderIdx()].myport)
                set_leaderAnswered(true);
            else {
                // TODO: Put locks into every cout!!
                cout << "ERROR: Received leader aliveness confirmation from process that isn't the leader" << endl;
            }
            // TODO: Check if there's something else
        }
    }
}


void* leader(void*){
    /*
     *  Thread responsible for checking current leader
     *
     */
    //cout << "In leader check thread" << endl;

    // TODO: lock!!
    set_isCheckingOnLeader(false);
    int delay = 0;
    while (1){
        // TODO: If I am not leader, send messafe to leader. Check if message was received
        if ((get_leaderIdx()) == -1 || (!(get_isOperational()))){ // NOTE: If I am the leader, leaderIdx is -1
            sleep(20);
            //cout<<"Leader Sleeping"<<endl;

        } else if ( (!(get_isCheckingOnLeader())) && ((get_electionSize()) == 0) && (!(get_isSilenced()))&&(!maior_ID)) {
            cout << "Will check on leader" << endl;

            // If I am not checking leader nor running an election, I should check the leader!
            set_isCheckingOnLeader(true);

            char buffer[messageLength];
            sprintf(buffer, "%i%s%i%s", m_vivo, delimiter,
                                        selfID, delimiter );

            cout << "Messaging leader. (" << buffer << ")" << endl;

            set_leaderAnswered(false);
            processes[get_leaderIdx()].sendMessage(buffer);
            //increasing_msgCount(m_vivo);
            increment_outMsgCount();

            sleep(5); // Wait to see if there'll be an answer
            //delay = rand() % 10 + 1;
            //sleep(delay);

            set_isCheckingOnLeader(false);

            // TODO: locks!!
            if ((!(get_leaderAnswered())) && (!(get_outElection()))){ // No answer from leader
                // TODO: Put timer to delay beginning

                cout << "No leader! Start election!" << endl;

                // Initiating election
                char outMsg[messageLength];
                sprintf( outMsg, "%i%s%i%s%i%s",
                         m_eleicao, delimiter,
                         selfID, delimiter,
                         selfElecValue, delimiter );

                set_isSilenced(false);

                // Broadcast of election message
                for (int i=0; i<N_PROC-1; i++){
                    processes[i].sendMessage(outMsg);
                    //increasing_msgCount(m_eleicao);
                }
                increment_outMsgCount();
                electionVectorMutex.lock();
                ongoingElections.push_back(selfID);
                electionVectorMutex.unlock();
                // Creating auxiliar thread
                pthread_t threadsAux;
                pthread_attr_t attrAux;

                pthread_attr_init(&attrAux);
                pthread_attr_setdetachstate(&attrAux, PTHREAD_CREATE_DETACHED);

                pthread_create(&threadsAux, &attrAux, electionFinish, NULL );
                // TODO: Check

            } // else { cout << "Leader ok" << endl; }
            sleep(10);
        } 
        else {
            sleep(10);
        }
    }
}


int main(int argc, char* argv[]){
    // int myServerPort = 8081;
    int sendPorts[] = {8080, 8081, 8082, 8083, 8084, 8085, 8086, 8087, 8088, 8089, 8090, 8091, 8092, 8093, 8094, 8095};
    messageLength = 1024;
    delimiter = new char[2];
    sprintf(delimiter, "\\");
    set_isOperational(true); // TODO: locks!
    // ----
    srand (time(NULL));
    cout << "Delimiter = " << delimiter << endl;

    // Function arguments
    if (argc < 4){
        cout << "ERROR: Enter PORT_NUMBER PROCESS_TOTAL CASE." << endl;
        return 1;
    }

    int myServerPort = atoi(argv[1]);
    N_PROC = atoi(argv[2]);
    caseNumber = atoi(argv[3]);
    if ( (myServerPort < 8080) || (myServerPort > 8084) ){
        cout << "ERROR: Enter a valid port number. Options are 8080, 8081, 8082, 8083 or 8084." << endl;
        return 1;
    }

    selfID = myServerPort;
    selfElecValue = myServerPort; // TODO: Change this (PID, for example)
    // ----

    if ( setupServerSocket(myServerPort) == -1 ) {
        cout << "ERROR: Could not setup socket" << endl;
        exit(1);
    }

    int idx = 0;
    for (int i = 0; i < N_PROC; i++) {
        if (sendPorts[i] == myServerPort){
            // Set initial leader
            if (myServerPort == 8080){
                set_leaderIdx(-1);
                cout << "Am initial leader!" << endl;
            }
            continue;
        }

        std::stringstream procName;
        procName << "Processo" << idx;
        ProcessClient Proc(sendPorts[i], procName.str());
        Proc.setupClientSocket();
        cout<<"Processo Criado"<<endl;

        processes.push_back(Proc);

        // Set initial leader
        if (sendPorts[i] == 8080){
            set_leaderIdx(idx);
            cout << "Initial leader: " << sendPorts[i] << endl;
        }

        idx++;
    }
    int THREAD_NUMBER=4;
    int threadResponse[THREAD_NUMBER];
    int threadStatus;
    pthread_t threads[THREAD_NUMBER];
    pthread_attr_t attr;
    void* status;

    // Initialize and set thread joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    cout << "Creating threads" << endl;

    threadResponse[0] = pthread_create(&threads[0], &attr, interface, NULL );
    threadResponse[1] = pthread_create(&threads[1], &attr, communication, NULL );
    threadResponse[2] = pthread_create(&threads[2], &attr, leader, NULL );
    threadResponse[3] = pthread_create(&threads[0], &attr, caseResolver, NULL );

    for( int i = 0; i < THREAD_NUMBER; i++ ) {
        if (threadResponse[i]){
        cout << "Error:unable to create thread," << threadResponse[i] <<"Thread Number:"<<i<<endl;
        exit(-1);
        }
    }

    // free attribute and wait for the other threads
    pthread_attr_destroy(&attr);
    for( int i = 0; i < THREAD_NUMBER; i++ ) {
       threadStatus = pthread_join(threads[i], &status);
       if (threadStatus) {
          cout << "Error:unable to join," << threadStatus << endl;
          exit(-1);
       }
       cout << "Main: completed thread " << i ;
       cout << "  Exiting with status :" << status << endl;
    }

    cout << "Main: program exiting." << endl;
}




// Auxiliary Functions ----------------------------------------------------------

int setupServerSocket(int port){
    /* Funtion to create a server socket
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


void* electionFinish(void*){
    /* Thread to wait out for election results and execute end of election procedures
     */
    int temp;
    int temp_after;
    mtx_vivo_ok.lock();
    temp=number_vivo_ok;
    mtx_vivo_ok.unlock();
    sleep(10);
    mtx_vivo_ok.lock();
    temp_after=number_vivo_ok;
    mtx_vivo_ok.unlock();
    if ((!(get_isSilenced()))&&(temp==temp_after)&&(!(get_alreadyOnThreadleader()))&&(!(get_outElection()))){
        // No OK message was received. Therefore I am the current leader

        set_alreadyOnThreadleader(true);
        set_leaderIdx(-1);

        char outMsg[messageLength];
        sprintf(outMsg, "%i%s%i%s", m_lider, delimiter,
                                    selfID, delimiter  );

        // Broadcast new leader message
        for (int i=0; i<N_PROC-1; i++){
            processes[i].sendMessage(outMsg);
            //increasing_msgCount(m_lider);
            
        }
        increment_outMsgCount();
        set_electionNumber();
    }

    pthread_exit(NULL);
}

void *caseResolver(void*){
    int numberOftimes = 0;
    int msgLiderbefore = 0;
    if(caseNumber==1){
        while(true){
            if ((get_leaderIdx()) == -1) {
                cout << "\t\t\t\t\t\t | Leader  |  " << selfID <<"|\tElection Number:"<<(get_electionNumber())<<endl;
            } 
            // else {
            //     cout << "\t\t\t\t\t\t | Leader  |  " << processes[get_leaderIdx()].myport<<"|\tElection Number:"<<(get_electionNumber())<<endl;
            // }
            numberOftimes = get_electionNumber();
            if(numberOftimes==10){
                cout<<"------------------------------------------------------------------------------------------------------"<<endl;
                cout<<"\t\t\tCompleted 10 Elections"<<endl;
                cout<<"\t\t\tTotal Msgs Election: "<<(get_totalElectionMsgs())<<endl;
                cout<<"------------------------------------------------------------------------------------------------------"<<endl;
                break;
            }
            else if(((get_leaderIdx()) == -1)&&(!(get_isCheckingOnLeader())))
            {   
                sleep(20);
                cout<<"\t******************************"<<endl;
                cout<<"\t* Lider : Simulating Failure *"<<endl;
                cout<<"\t******************************"<<endl;
                int diff = 0;
                set_isOperational(false);
                int count=0;
                mtx_msgCount.lock();
                msgLiderbefore = msgCount[m_lider];
                mtx_msgCount.unlock();
                while(true){
                    sleep(40);
                    mtx_msgCount.lock();
                    diff = (msgCount[m_lider] - msgLiderbefore);
                    mtx_msgCount.unlock();
                    if (diff>0){
                        cout<<"\t******************************"<<endl;
                        cout<<"\t*     Recovering Failure     *"<<endl;
                        cout<<"\t******************************"<<endl;
                        set_isOperational(true);
                        break;
                    }
                    else if(count==20){
                        cout<<"Too many tries"<<endl;
                        break;
                    }
                    else{
                        cout<<"Lider not decided"<<endl;
                        cout<<"Waiting for decision"<<endl;
                        sleep(10);
                    }
                    count++;
                }
            }
            sleep(2);
        }
    }
    if(caseNumber==2){
        int onfailure;
        int temp_numberOftimes=0;
        int numberMsgs=0;
         while(true){
            if ((get_leaderIdx()) == -1) {
                cout << "\t\t\t\t\t\t | Leader  |  " << selfID <<"|\tElection Number:"<<(get_electionNumber())<<endl;
            } else {
                cout << "\t\t\t\t\t\t | Leader  |  " << processes[get_leaderIdx()].myport<<"|\tElection Number:"<<(get_electionNumber())<<endl;
            }
            numberMsgs = get_inMsgCount()+get_outMsgCount();
            sleep(10);
            if((get_leaderIdx()) == -1)
            {   
                sleep(10);
                set_isOperational(false);
                cout<<"\t******************************"<<endl;
                cout<<"\t* Lider : Simulating Failure *"<<endl;
                cout<<"\t******************************"<<endl;
                onfailure=1;
                sleep(10);
                
            }
            if(onfailure==1){
                cout<<"\t******************************"<<endl;
                cout<<"\t*      Simulating Failure    *"<<endl;
                cout<<"\t******************************"<<endl;
            }
            sleep(10);
            temp_numberOftimes = get_inMsgCount()+get_outMsgCount();
            if(temp_numberOftimes==numberMsgs){
                cout<<"------------------------------------------------------------------------------------------------------"<<endl;
                cout<<"\t\t\tCompleted Elections"<<"|\tElection Number:"<<(get_electionNumber())<<endl;
                cout<<"\t\t\tTotal Msgs Election: "<<(get_totalElectionMsgs())<<endl;
                cout<<"------------------------------------------------------------------------------------------------------"<<endl;
                break;
            }
            sleep(2);
        }

    }
    


    

    
}


// ProcessClient class ----------------------------------------------------------
ProcessClient::ProcessClient(int port, string name){
    myPid = getpid();
    processName = name;
    myport = port;
}

int ProcessClient::getPid(){ // NOTE: What is the usage of this?
    return (int)myPid;
}

int ProcessClient::sendMessage(char client_buffer[]){
    // TODO: Comment this out
    cout << "Sending message '" << client_buffer << "' to process " << myport << endl;

    socklen_t addrlen = sizeof(remaddr);
    memset((char *) &remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(myport);
    char server[] = "127.0.0.1";
    int recvlen;
    if (inet_aton(server, &remaddr.sin_addr)==0){
        fprintf(stderr, "inet_aton() failed\n");
        return 1;
	  }

    if (sendto(client_socket_ID, client_buffer, strlen(client_buffer), 0, (struct sockaddr *)&remaddr, addrlen)==-1) {
        perror("sendto");
        return 1;
	  }

    return 0; // If everything went fine
}

int ProcessClient::setupClientSocket(){
    /*
     *  Thread responsible for create a client socket
     *
     */
    //struct sockaddr_in myaddr;
    int port = myport;
    int client_socket;
    int recvlen;		/* # bytes in acknowledgement message */
    char server[] = "127.0.0.1";	/* change this to use a different server */
    if ((client_socket=socket(AF_INET, SOCK_DGRAM, 0))==-1){
        printf("socket created\n");
    }
    memset( (char *)&myaddr, 0, sizeof(myaddr) );
	  myaddr.sin_family = AF_INET;
	  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    if ( bind(client_socket, (struct sockaddr *)&myaddr,
              sizeof(myaddr)) < 0 ){
        perror("bind failed");
        exit(0);
    }
    //client_sockets.push_back(client_socket);
    client_socket_ID=client_socket;
    return 1;
}
//----------------------------------------
int get_electionSize(){
    int size = 0;
    electionVectorMutex.lock();
    size = ongoingElections.size();
    electionVectorMutex.unlock();
    return size;
}
int erase_ongoingElections(){
    electionVectorMutex.lock();
    for (int i = 0; i < ongoingElections.size(); i++){
        ongoingElections.erase( ongoingElections.begin()+i );

    }
    electionVectorMutex.unlock();
}
//----------------------------------------
void set_isCheckingOnLeader(bool temp){
    mtx_isCheckingOnLeader.lock();
    isCheckingOnLeader = temp ;
    mtx_isCheckingOnLeader.unlock();

    
}
bool get_isCheckingOnLeader(){
    bool temp;
    mtx_isCheckingOnLeader.lock();
    temp  = isCheckingOnLeader;
    mtx_isCheckingOnLeader.unlock();
    return temp;
}
//----------------------------------------
void set_isOperational(bool temp){
    mtx_isOperational.lock();
    isOperational = temp ;
    mtx_isOperational.unlock();
}
bool get_isOperational(){
    bool temp;
    mtx_isOperational.lock();
    temp  = isOperational;
    mtx_isOperational.unlock();
    return temp;
}    
//----------------------------------------
void set_leaderIdx(int temp){
    mtx_leaderIdx.lock();
    leaderIdx = temp ;
    mtx_leaderIdx.unlock();
}
int get_leaderIdx(){
    int temp;
    mtx_leaderIdx.lock();
    temp  = leaderIdx;
    mtx_leaderIdx.unlock();
    return temp;
}    
//----------------------------------------
void set_isSilenced(bool temp){
    mtx_leaderIdx.lock();
    isSilenced = temp ;
    mtx_leaderIdx.unlock();
}
bool get_isSilenced(){
    bool temp;
    mtx_leaderIdx.lock();
    temp  = isSilenced;
    mtx_leaderIdx.unlock();
    return temp;
} 
//----------------------------------------
void set_outElection(bool temp){
    mtx_outElection.lock();
    outElection = temp ;
    mtx_outElection.unlock();
}
bool get_outElection(){
    bool temp;
    mtx_outElection.lock();
    temp  = outElection;
    mtx_outElection.unlock();
    return temp;
}  
//----------------------------------------
void set_leaderAnswered(bool temp){
    mtx_leaderAnswered.lock();
    leaderAnswered = temp;
    mtx_leaderAnswered.unlock();
}
bool get_leaderAnswered(){
    bool temp;
    mtx_leaderAnswered.lock();
    temp  = leaderAnswered;
    mtx_leaderAnswered.unlock();
    return temp;
}
//----------------------------------------
void increment_outMsgCount(){
    mtx_outMsgCount.lock();
    outMsgCount++;
    mtx_outMsgCount.unlock();
}
int get_outMsgCount(){
    int temp_count;
    mtx_outMsgCount.lock();
    temp_count = outMsgCount;
    mtx_outMsgCount.unlock();
    return temp_count;
}
//----------------------------------------
void increment_inMsgCount(){
    mtx_inMsgCount.lock();
    inMsgCount++;
    mtx_inMsgCount.unlock();
}
int get_inMsgCount(){
    int temp_count;
    mtx_inMsgCount.lock();
    temp_count = inMsgCount;
    mtx_inMsgCount.unlock();
    return temp_count;
}
//----------------------------------------
void increasing_msgCount(int temp){
    mtx_msgCount.lock();
    msgCount[temp]++;
    mtx_msgCount.unlock();
}
//----------------------------------------
void set_totalElectionMsgs(int temp){
    mtx_totalElectionMsgs.lock();
    totalElectionMsgs = temp;
    mtx_totalElectionMsgs.unlock();
}
int get_totalElectionMsgs(){
    int temp;
    mtx_totalElectionMsgs.lock();
    temp  = totalElectionMsgs;
    mtx_totalElectionMsgs.unlock();
    return temp;
}

//----------------------------------------
void set_electionNumber(){
    mtx_electionNumber.lock();
    electionNumber=electionNumber+1;
    mtx_electionNumber.unlock();
}
int get_electionNumber(){
    int temp;
    mtx_electionNumber.lock();
    temp  = electionNumber;
    mtx_electionNumber.unlock();
    return temp;
}
//----------------------------------------
void set_alreadyOnThreadleader(int temp){
    mtx_alreadyOnThreadleader.lock();
    alreadyOnThreadleader = temp;
    mtx_alreadyOnThreadleader.unlock();
}
int get_alreadyOnThreadleader(){
    int temp;
    mtx_alreadyOnThreadleader.lock();
    temp  = alreadyOnThreadleader;
    mtx_alreadyOnThreadleader.unlock();
    return temp;
}

//gnome-terminal --tab --title="tab 1" --command="./Process 8080 4 1" --tab --title="Process 2 " --command="./Process 8081 4 1" --tab --title="Process 3 " --command="./Process 8082 4 1" --tab --title="Process 4 " --command="./Process 8083 4 1"
