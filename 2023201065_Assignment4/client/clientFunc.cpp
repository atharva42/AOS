#include "client.h"



void clearLogs() {
    int fd = open(logFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd != -1) {
        close(fd);
    } else {
        std::cerr << "Failed to clear the log file." << std::endl;
    }
}

void writeLog(const string &text ){
    int fd;
    int attempts = 0;

    while (attempts < 3) { 
        fd = open(logFile.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);

        if (fd != -1) {
            const char* to_write = text.c_str();
            ssize_t bytes_written = write(fd, to_write, strlen(to_write));

            if (bytes_written == -1) {
                std::cerr << "Failed to write to the log file." << std::endl;
            } else {
                close(fd);
                return;
            }
        }

        attempts++;
    }
}

vector<string> splitString(string s, string delim=":")
{
    vector<string> result;
    size_t start = 0;

    for (size_t end = s.find(delim, start); end != string::npos; end = s.find(delim, start)) {
        string t = s.substr(start, end - start);
        result.push_back(t);
        start = end + delim.length();
    }

    result.push_back(s.substr(start));

    return result;
}


vector<string> readFromFile(char* path){
    fstream trackerInfoFile;
    trackerInfoFile.open(path, ios::in);

    vector<string> res;
    if(trackerInfoFile.is_open()){
        string t;
        while(getline(trackerInfoFile, t)){
            res.push_back(t);
        }
        trackerInfoFile.close();
    }
    else{
        cout << "Tracker Info file not found.\n";
        exit(-1);
    }
    return res;
}

void handleArgs(int argc, char *argv[]){
    string clientInfo = argv[1];
    string trackerInfoFilename = argv[2];
    logFile = clientInfo+"_log.txt";
    clearLogs();
    vector<string> serverAddress = splitString(clientInfo);
    clientIP = serverAddress[0];
    clientPort = stoi(serverAddress[1]);
    char curDir[128];
    getcwd(curDir, 128);
    string path = string(curDir);
    path+="/"+trackerInfoFilename;
    vector<string> serverInfo = readFromFile(&path[0]);
    trackerIP1 = serverInfo[0];
    tracker1_port = stoi(serverInfo[1]);
    trackerIP2 = serverInfo[2];
    tracker2_port = stoi(serverInfo[3]);
    writeLog("Peer Address : " + string(clientIP)+ ":" +to_string(clientPort));
    writeLog("Tracker 1 Address: "+string(trackerIP1)+": "+to_string(tracker1_port));
    writeLog("Tracker 2 Address: "+string(trackerIP2)+": "+to_string(tracker2_port));
    writeLog("Log file name : " + string(logFile) + "\n");
}

int connectToTracker(int trackerNum, struct sockaddr_in &serverAddress, int sock){
    char* curTrackIP;
    uint16_t curTrackPort;
    if(trackerNum == 1){
        curTrackIP = &trackerIP1[0]; 
        curTrackPort = tracker1_port;
    }
    else{
        curTrackIP = &trackerIP2[0]; 
        curTrackPort = tracker2_port;
    }
    bool error = 0;

    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(curTrackPort); 
       
    if(inet_pton(AF_INET, curTrackIP, &serverAddress.sin_addr)<=0)  { 
        error = 1;
    } 
    if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) { 
        error = 1;
    } 
    if(error){
        if(trackerNum == 1)
            return connectToTracker(2, serverAddress, sock);
        else
            return -1;
    }
    writeLog("connected to server " + to_string(curTrackPort));
    return 0;
}