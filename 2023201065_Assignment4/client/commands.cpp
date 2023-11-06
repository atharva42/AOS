#include "client.h"

int list_groups(int socket){
    char dum[5];
    strcpy(dum, "test");
    write(socket, dum, 5);

    char serverreply[3*SIZE];
    memset(serverreply, 0, sizeof(serverreply));
    read(socket, serverreply, 3*SIZE);
    writeLog("list of groups reply: " + string(serverreply));

    vector<string> list_grps = splitString(string(serverreply), "$$");

    for(size_t i=0;i<list_grps.size()-1;i++){
        cout<<list_grps[i]<<endl;
    }
    return 0;
}

int list_requests(int socket){
    writeLog("waiting for response");

    char dum[5];
    strcpy(dum, "test");
    write(socket, dum, 5);
    
    char serverreply[3*SIZE];
    memset(serverreply, 0, 3*SIZE);
    read(socket, serverreply, 3*SIZE);
    if(string(serverreply) == "**err**") return -1;
    if(string(serverreply) == "**er2**") return 1;
    writeLog("request list: " + string(serverreply));

    vector<string> requestsQueue = splitString(string(serverreply), "$$");
    writeLog("list request response size: "+ to_string(requestsQueue.size()));
    for(size_t i=0; i<requestsQueue.size()-1; i++){
        cout << requestsQueue[i] << endl;
    }
    return 0;
}

void accept_request(int socket){
    char dum[5];
    strcpy(dum, "test");
    write(socket, dum, 5);

    char buffer[96];
    read(socket, buffer, 96);
    cout<<buffer<<endl;
}

void leave_group(int socket){
    writeLog("waiting for response");
    char buffer[96];
    read(socket, buffer, 96);
    cout<<buffer<<endl;
}

int processCMD(vector<string> Input, int sock){
    char serverreply[10240]; 
    bzero(serverreply, 10240);
    read(sock , serverreply, 10240); 
    cout << serverreply << endl;
    writeLog("primary server response: " + string(serverreply));
 
    if(string(serverreply) == "Invalid argument count") return 0;
    if(Input[0] == "login"){
        if(string(serverreply) == "Login Successful"){
            loggedIn = true;
            string clientAddress = clientIP + ":" + to_string(clientPort);
            write(sock, &clientAddress[0], clientAddress.length());
        }
    }
    else if(Input[0] == "logout"){
        loggedIn = false;
    }
    else if(Input[0] == "list_groups"){
        return list_groups(sock);
    }
    else if(Input[0] == "list_requests"){
        int t;
        if((t = list_requests(sock)) < 0){
            cout << "You are not the admin of this group\n";
        }
        else if(t>0){
            cout << "No pending requests\n";
        }
        else return 0;
    }
    else if(Input[0] == "accept_request"){
        accept_request(sock);
    }
    else if(Input[0] == "leave_group"){
        leave_group(sock);
    }
    return 0;
}