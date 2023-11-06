#ifndef SERVER_HEADER 
#define SERVER_HEADER

#include <bits/stdc++.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <signal.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <stdarg.h> 
#include <errno.h> 
#include <fcntl.h>
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
using namespace std;

#define SA struct sockaddr 
#define TRACKER_PORT 18000
#define ll long long int
#define MAXLINE 4096 

extern string logFile, trackerIP1, trackerIP2,curTrackerIP,seederFileName;
extern uint16_t trackerPort_1,trackerPort_2, curTrackerPort;
extern unordered_map<string, bool> isLoggedIn;
extern unordered_map<string, string> loginCreds;
extern unordered_map<string, string> fileSize;
extern unordered_map<string, string> grpAdmins;
extern vector<string> allGroups;
extern unordered_map<string, set<string>> groupMembers;
extern unordered_map<string, set<string>> grpPendngRequests;
extern unordered_map<string, string> unameToPort;

void handle_connection(int);
void leave_group(vector<string>, int, string);
void accept_request(vector<string>, int, string);
void list_requests(vector<string>, int, string);
void join_group(vector<string>, int, string);
void list_groups(vector<string>, int);
int create_group(vector<string>, int, string);
int validateLogin(vector<string>);
int createUser(vector<string>);
void connectionHandler(int);
vector<string> readfromFile(char*);
void writeLog(const string &);
void clearLogs();
void* check_input(void*);
void handleArgs(int, char **);
bool checkPaths(const string &s);
vector<string> splitString(string, string);

#endif