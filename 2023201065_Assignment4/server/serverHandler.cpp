#include "server.h"

int createUser(vector<string> Input){
    string userName = Input[1];
    string passkey = Input[2];

    if(loginCreds.find(userName) == loginCreds.end()){
        loginCreds.insert({userName, passkey});
    }
    else{
        return -1;
    }
    return 0;
}

int create_group(vector<string> Input, int clientSocket, string clientUid){
    if(Input.size() != 2){
        write(clientSocket, "Invalid argument count", 22);
        return -1;
    }
    for(auto i: allGroups){
        if(i == Input[1]) return -1;
    }
    grpAdmins.insert({Input[1], clientUid});
    allGroups.push_back(Input[1]);
    groupMembers[Input[1]].insert(clientUid);
    return 0;
}

void join_group(vector<string> Input, int clientSocket, string clientUid){
    if(Input.size()!=2){
        write(clientSocket, "Invalid argument count", 22);
    }
    writeLog("join_group function ..");
    if(grpAdmins.find(Input[1]) == grpAdmins.end()){
        write(clientSocket, "Invalid group ID.", 18);
    }
    else if(groupMembers[Input[1]].find(clientUid) == groupMembers[Input[1]].end()){
        grpPendngRequests[Input[1]].insert(clientUid);
        write(clientSocket, "Group request sent", 18);
    }
    else{
        write(clientSocket, "You are already in this group", 30);
    }
}

void list_groups(vector<string>Input, int clientSocket){
    if(Input.size()!=1){
        write(clientSocket, "Invalid argument count", 22);
        return;
    }
    write(clientSocket, "All groups:", 11);

    char dum[5];
    string allgrps = "";
    read(clientSocket, dum, 5);
    if(allGroups.size()==0){
        write(clientSocket, "No groups found$$", 18);
        return;
    }
    for(size_t i=0; i<allGroups.size(); i++){
        allgrps += allGroups[i] + "$$";
    }
    write(clientSocket, &allgrps[0], allgrps.length());
}

void accept_request(vector<string> Input, int clientSocket, string clientUid){
    if(Input.size()!=3){
        write(clientSocket, "Invalid argument count", 22);
        return;
    }
    write(clientSocket, "Accepting request...", 21);

    char dum[5];
    read(clientSocket, dum, 5);
    if(grpAdmins.find(Input[1]) == grpAdmins.end()){
        writeLog("Invalid group ID");
        write(clientSocket, "Invalid group ID.", 18);
    }
    else if(grpAdmins.find(Input[1])->second == clientUid){
        for(auto i: grpPendngRequests[Input[1]]){
            writeLog(i);
        }
        grpPendngRequests[Input[1]].erase(Input[2]);
        groupMembers[Input[1]].insert(Input[2]);
        writeLog("Group request accepted");
        write(clientSocket, "Request accepted.", 18);
    }
    else{
        write(clientSocket, "You are not the admin of this group", 35);
    }
}
void list_requests(vector<string> Input, int clientSocket, string clientUid){
    if(Input.size()!=2){
        write(clientSocket, "Invalid argument count", 22);
        return;
    }
    write(clientSocket, "Fetching group requests...", 27);
    char dum[5];
    read(clientSocket, dum, 5);
    if(grpAdmins.find(Input[1]) == grpAdmins.end() || grpAdmins[Input[1]] != clientUid){
        writeLog("list requests");
        write(clientSocket, "**err**", 7);
    }
    else if(grpPendngRequests[Input[1]].size() == 0){
        write(clientSocket, "**er2**", 7);
    }
    else {
        string serverreply = "";
        writeLog("pending request size: "+  to_string(grpPendngRequests[Input[1]].size()));
        for(auto i = grpPendngRequests[Input[1]].begin(); i!= grpPendngRequests[Input[1]].end(); i++){
            serverreply += string(*i) + "$$";
        }
        write(clientSocket, &serverreply[0], serverreply.length());
        writeLog("reply :" + serverreply);
    }
}

void leave_group(vector<string> Input, int clientSocket, string clientUid){
    if(Input.size()!=2){
        write(clientSocket, "Invalid argument count", 22);
        return;
    }
    write(clientSocket, "Leaving group...", 17);
    if(grpAdmins.find(Input[1]) == grpAdmins.end()){
        writeLog("Invalid group ID");
        write(clientSocket, "Invalid group ID.", 18);
    }
    else if(groupMembers[Input[1]].find(clientUid) != groupMembers[Input[1]].end()){
        if(grpAdmins[Input[1]] == clientUid){
            write(clientSocket, "You are the admin of this group, you cant leave!", 48);
        }
        else{
            groupMembers[Input[1]].erase(clientUid);
            writeLog("Successfully left the group");
            write(clientSocket, "Group left succesfully", 23);
        }
    }
    else{
        writeLog("You are not part of this grp");
        write(clientSocket, "You are not in this group", 26);
    }
}

int validateLogin(vector<string> Input){
    string userName = Input[1];
    string passkey = Input[2];

    if(loginCreds.find(userName) == loginCreds.end() || loginCreds[userName] != passkey){
        return -1;
    }

    if(isLoggedIn.find(userName) == isLoggedIn.end()){
        isLoggedIn.insert({userName, true});
    }
    else{
        if(isLoggedIn[userName]){
            return 1;
        }
        else{
            isLoggedIn[userName] = true;
        }
    }
    return 0;
}

void connectionHandler(int clientSocket){
    string clientUid = "";
    string clientGid = "";
    writeLog("Server started for client socket number " + to_string(clientSocket));
    for(;;){
        char InputLine[1024] = {0};
        int fd = read(clientSocket, InputLine, 1024) ;
        if(fd<= 0){
            isLoggedIn[clientUid] = false;
            close(clientSocket);
            break;
        }
        writeLog("client request: "+string(InputLine));
        string inFlow = string(InputLine);
        stringstream ss(inFlow);
        vector<string> Inputs;
        for (string s; ss >> s;) {
    Inputs.push_back(s);
}
        if(Inputs[0] == "create_user"){
            if(Inputs.size() != 3){
                write(clientSocket, "Invalid argument count", 22);
            }
            else{
                if(createUser(Inputs) < 0){
                    write(clientSocket, "User exists", 11);
                }
                else{
                    write(clientSocket, "Account created", 15);
                    writeLog("User account created!!");
                }
            }
        }
        else if(Inputs[0] == "login"){
            if(Inputs.size() != 3){
                write(clientSocket, "Invalid argument count", 22);
            }
            else{
                int r;
                if((r = validateLogin(Inputs)) < 0){
                    write(clientSocket, "Username/password incorrect", 28);
                }
                else if(r > 0){
                    write(clientSocket, "You already have one active session", 35);
                }
                else{
                    write(clientSocket, "Login Successful", 16);
                    clientUid = Inputs[1];
                    char buf[96];
                    read(clientSocket, buf, 96);
                    string peerAddress = string(buf);
                    unameToPort[clientUid] = peerAddress;
                    writeLog("User: "+Inputs[1]+" logged in.");
                }
            }
                        
        }
        else if(Inputs[0] ==  "logout"){
            isLoggedIn[clientUid] = false;
            write(clientSocket, "Logout Successful", 17);
            writeLog("User: logged out.");
        }
        else if(Inputs[0] == "create_group"){
            if(create_group(Inputs, clientSocket, clientUid) >=0){
                clientGid = Inputs[1];
                write(clientSocket, "Group created", 13);
                writeLog(Inputs[1]+" group created");
            }
            else{
                write(clientSocket, "Group exists", 12);
            }
        }
        else if(Inputs[0] == "list_groups"){
            list_groups(Inputs, clientSocket);
        }
        else if(Inputs[0]=="join_group"){
            join_group(Inputs, clientSocket, clientUid);
        }
        else if(Inputs[0] == "list_requests"){
            list_requests(Inputs, clientSocket, clientUid);
        }
        else if(Inputs[0] == "accept_request"){
            accept_request(Inputs, clientSocket, clientUid);
        }
        else if(Inputs[0] == "leave_group"){
            leave_group(Inputs, clientSocket, clientUid);
        }
        else{
            write(clientSocket, "Invalid command", 16);
            writeLog("Invalid command was invoked");
        }

    }
    writeLog("Server started for client socket number " + to_string(clientSocket));
    close(clientSocket);
}