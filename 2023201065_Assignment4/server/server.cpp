#include "server.h"

string logFile, trackerIP1, trackerIP2, curTrackerIP;
uint16_t trackerPort_1, trackerPort_2, curTrackerPort;
unordered_map<string, string> loginCreds;
unordered_map<string, bool> isLoggedIn;
unordered_map<string, string> fileSize;
unordered_map<string, string> grpAdmins;
vector<string> allGroups;
unordered_map<string, set<string>> groupMembers;
unordered_map<string, set<string>> grpPendngRequests;
unordered_map<string, string> unameToPort;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Please provide all the arguments";
        return -1;
    }
    handleArgs(argc, argv);
    int option = 1;
    int serverSocket;
    struct sockaddr_in serverAddress;
    int addlen = sizeof(serverAddress);
    pthread_t exitDetectionThreadId;

    serverSocket= socket(AF_INET, SOCK_STREAM, 0);
    if ((serverSocket == 0))
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    writeLog("Server Socket created");
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(curTrackerPort);

    if (inet_pton(AF_INET, &curTrackerIP[0], &serverAddress.sin_addr) <= 0)
    {
        printf("Wrong Server Address");
        return -1;
    }
    if (bind(serverSocket, (SA *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    writeLog("Binding completed");

    if (listen(serverSocket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    writeLog("Listening for connections.....");
    vector<thread> vectorThread;
    if (pthread_create(&exitDetectionThreadId, NULL, check_input, NULL) == -1)
    {
        perror("pthread");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        int clientSocket= accept(serverSocket, (SA *)&serverAddress, (socklen_t *)&addlen);
        if (clientSocket < 0)
        {
            perror("Acceptance error");
            writeLog("Error in accepting connection");
        }
        writeLog("Connection accepted");
        vectorThread.push_back(thread(connectionHandler, clientSocket));
    }
    for (auto i = vectorThread.begin(); i != vectorThread.end(); i++)
    {
        if (i->joinable())
            i->join();
    }
    writeLog("EXITING");
    return 0;
}