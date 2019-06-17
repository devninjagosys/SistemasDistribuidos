#include <iostream>
#include <string>

using namespace std;
int main(int argc, char* argv[]){
    // int myServerPort = 8081;
    int sendPorts[] = {8080, 8081, 8082, 8083, 8084, 8085, 8086, 8087, 8088, 8089, 8090, 8091, 8092, 8093, 8094, 8095};

    // Function arguments
    if (argc < 2){
        cout << "ERROR: Enter PROCESS_TOTAL CASE." << endl;
        return 1;
    }
    int PROCESS_TOTAL = atoi(argv[1]);
    int CASE = atoi(argv[2]);
    string command="gnome-terminal";
    for(int i=0;i<PROCESS_TOTAL;i++){
        command=command+" --tab --title=\"tab"+std::to_string(i)+"\""+" --command=\"./Process "+std::to_string(sendPorts[i])+" "+std::to_string(PROCESS_TOTAL)+" "+std::to_string(CASE)+"\"";

    }
    std::cout<<command<<std::endl;
    return 0;
    }


//gnome-terminal --tab --title="tab 1" --command="./Process 8080 4 1" --tab --title="Process 2 " --command="./Process 8081 4 1" --tab --title="Process 3 " --command="./Process 8082 4 1" --tab --title="Process 4 " --command="./Process 8083 4 1"
