#include "client.h"


void handleClientRequest(int clientSocket){
    string clientUid = "";

    writeLog("\nclient socket num: " + to_string(clientSocket) + "\n");
    char inptline[1024] = {0}; 

    if(read(clientSocket , inptline, 1024) <=0){
        close(clientSocket);
        return;
    }
    
    writeLog("client request at server " + string(inptline));
    vector<string> inpt = splitString(string(inptline), "$$");
    writeLog(inpt[0]);

    if(inpt[0] == "get_file_path"){
        string filepath = fileToFilePath[inpt[1]];
        writeLog("command from peer client: " +  string(inptline));
        write(clientSocket, &filepath[0], strlen(filepath.c_str()));
    }
    close(clientSocket);
    return;
}

void* runAsServer(void* arg){
    int serverSocket;
    struct sockaddr_in serverAddress;
    int addlen = sizeof(serverAddress);
    int option =1;

    writeLog("\n"+to_string(clientPort) + " will start running as server");
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0))==0){
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    writeLog("Server socket created");
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))){
        perror("setsockopt"); 
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_port = htons(clientPort);
    if(inet_pton(AF_INET, &clientIP[0], &serverAddress.sin_addr)<=0)  { 
        printf("\nInvalid address/ Address not supported \n"); 
        return NULL; 
    } 
       
    if (bind(serverSocket, (SA *)&serverAddress,  sizeof(serverAddress))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    writeLog(" Binding completed.");
     if (listen(serverSocket, 3) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    writeLog("Listening...\n");
    vector<thread> vThread;
    while(true){
        int clientSocket;
        if((clientSocket = accept(serverSocket, (SA *)&serverAddress, (socklen_t *)&addlen)) < 0){
            perror("Acceptance error");
            writeLog("Error in accept"); 
        }
        writeLog(" Connection Accepted");
        vThread.push_back(thread(handleClientRequest, clientSocket));
    }
    for(auto it=vThread.begin(); it!=vThread.end();it++){
        if(it->joinable()) it->join();
    }
    close(serverSocket);
}

string connectToPeer(char* serverClientIP, char* serverPortIP, string task){
    int clientSocket = 0;
    struct sockaddr_in clientServerAdd; 

    writeLog("\nInside connectToPeer");

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
        printf("\n Socket creation error \n"); 
        return "error"; 
    } 
    writeLog("Socket Created");

    clientServerAdd.sin_family = AF_INET; 
    uint16_t clientPort = stoi(string(serverPortIP));
    clientServerAdd.sin_port = htons(clientPort); 
    writeLog("\n needs to connect to " + string(serverClientIP) + ":" + to_string(clientPort));

    if(inet_pton(AF_INET, serverClientIP, &clientServerAdd.sin_addr) < 0){ 
        perror("Peer Connection Error(INET)");
    } 
    if (connect(clientSocket, (struct sockaddr *)&clientServerAdd, sizeof(clientServerAdd)) < 0) { 
        perror("Peer Connection Error");
    }
    writeLog("Connected to peer " + string(serverClientIP) + ":" + to_string(clientPort));

    string curcmd = splitString(task, "$$").front();
    writeLog("current command " + curcmd);
    if(curcmd == "get_file_path"){
        if(send(clientSocket , &task[0] , strlen(&task[0]) , MSG_NOSIGNAL ) == -1){
            printf("Error: %s\n",strerror(errno));
            return "error"; 
        }
        char serverreply[10240] = {0};
        if(read(clientSocket, serverreply, 10240) < 0){
            perror("err: ");
            return "error";
        }
        writeLog("server reply for get file path:" + string(serverreply));
        fileToFilePath[splitString(task, "$$").back()] = string(serverreply);
    }
    close(clientSocket);
    writeLog("terminating connection with " + string(serverClientIP) + ":" + to_string(clientPort));
    return "aa";
}