#include "server.h"

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

bool checkPaths(const string &s){
  struct stat buffer;
  return (stat (s.c_str(), &buffer) == 0);
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

vector<string> readfromFile(char *path)
{
    fstream serverInfoFile;
    serverInfoFile.open(path, ios::in);
    vector<string> finalvalue;
    if (!serverInfoFile.is_open())
    {
        cout << "unable to locate server info file";
        exit(-1);
    }
    else
    {
        string s;
        while (getline(serverInfoFile, s))
        {
            finalvalue.push_back(s);
        }
        serverInfoFile.close();
    }
    return finalvalue;
}

void handleArgs(int argc, char *argv[])
{
    logFile = "tracker_log" + string(argv[2]) + ".txt";
    string path = argv[2];
    clearLogs();
    vector<string> trackerDetails = readfromFile(argv[1]);

    if (string(argv[2]) == "1")
    {
        trackerIP1 = trackerDetails[0];
        trackerPort_1 = stoi(trackerDetails[1]);
        curTrackerIP = trackerIP1;
        curTrackerPort = trackerPort_1;
    }
    else
    {
        trackerIP2 = trackerDetails[2];
        trackerPort_2 = stoi(trackerDetails[3]);
        curTrackerIP = trackerIP2;
        curTrackerPort = trackerPort_2;
    }
    writeLog("Tracker Details for the session are saved below: ");
    writeLog("Tracker 1 Address: " + string(trackerIP1) + " and port no " + to_string(trackerPort_1));
    writeLog("Tracker 2 Address: " + string(trackerIP2) + "and port no " + to_string(trackerPort_2));
}

void *check_input(void *arg)
{
    for(;;)
    {
        string inputData;
        getline(cin, inputData);
        if (inputData == "quit")
        {
            exit(0);
        }
    }
}