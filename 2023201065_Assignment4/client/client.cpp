#include "client.h"

string logFile, trackerIP1, trackerIP2,curTrackerIP, clientIP, seederFileName;
uint16_t clientPort, tracker1_port, tracker2_port, curTrackerPort;
bool loggedIn;
unordered_map<string, string> fileToFilePath;

int main(int argc, char* argv[]){
    if(argc != 3){
        cout << "Give arguments as <peer IP:port> and <tracker info file name>\n";
        return -1;
    }
    handleArgs(argc, argv);

    int sock = 0;
    struct sockaddr_in serverAddress;
    pthread_t threadServer;
    if((sock = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("\n Socket creation error \n"); 
        return -1;
    }
    writeLog("Client Socket created");
    if(pthread_create(&threadServer,  NULL, runAsServer, NULL) == -1){
        perror("pthread"); 
        exit(EXIT_FAILURE); 
    }
    if(connectToTracker(1, serverAddress, sock) < 0){
        exit(-1); 
    }
    while(true){
        cout<<">>";
        string InputLine, s;
        getline(cin, InputLine);
        if(InputLine.length()<1) continue;

        stringstream ss(InputLine);
        vector<string> Inputs;
        while(ss>>s){
            Inputs.push_back(s);
        }
        //login and stuff ahead
        if(Inputs[0] == "login" && loggedIn){
            cout << "You already have one active session" << endl;
            continue;
        }
        if(Inputs[0] != "login" && Inputs[0] != "create_user" && !loggedIn){
             cout << "Please login / create an account" << endl;
                continue;
        }

        if(send(sock , &InputLine[0] , strlen(&InputLine[0]) , MSG_NOSIGNAL ) == -1){
            printf("Error: %s\n",strerror(errno));
            return -1;
        }
        writeLog("sent to server: " + Inputs[0]);
        processCMD(Inputs, sock);
    }
    close(sock);
    return 0;
    
}