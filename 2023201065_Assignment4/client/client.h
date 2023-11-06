#ifndef CLIENT_HEADER 
#define CLIENT_HEADER

#include <iostream>
#include <openssl/sha.h>
#include<bits/stdc++.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <pthread.h>
#include<fcntl.h>
#include <fstream>
using namespace std;

#define SA struct sockaddr 
#define SIZE 32768
#define ll long long int
#define FILE_SEGMENT_SZ 524288

extern string logFile, trackerIP1, trackerIP2,curTrackerIP,clientIP, seederFileName;
extern uint16_t clientPort, tracker1_port, tracker2_port, curTrackerPort;
extern bool loggedIn;
extern unordered_map<string, string> fileToFilePath;

void handleArgs(int, char **);
void handleClientRequest(int);
void clearLogs();
void writeLog(const string &);
vector<string> splitString(string, string);
vector<string> getTrackerInfo(char*);
vector<string> readFromFile(char*);
void* runAsServer(void*);
void handleClientRequest(int);
string connectToPeer(char*, char*, string);
int list_groups(int);
int list_requests(int);
void accept_request(int);
void leave_group(int);
int processCMD(vector<string>, int);
int connectToTracker(int, struct sockaddr_in &, int);

#endif