#include<iostream>
#include<cstring>
#include<string>
#include<filesystem>
#include<dirent.h>
#include<vector>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
using namespace std;
namespace fs = std::filesystem;

string prev_path="getenv";

//this function keeps the terminal interactive

void displayOnScreen(string cwd){
    string cwd1="";
    string username = getenv("USER");
    char hostname[_SC_HOST_NAME_MAX+1];
    gethostname(hostname, _SC_HOST_NAME_MAX+1);
    string home_path=getenv("HOME");
    size_t pos = cwd.find(home_path);
    cout << username << "@" << hostname << ":";
    if (pos == string::npos) {
        cout << cwd<<" ";
    } else {
        cout << "~" << cwd.substr(pos + home_path.length())<<"> ";
    }
}

//this function creates tokens of given user input

vector<string> tokeniser(){
    char raws[1000000];
    cin.getline(raws, sizeof(raws));
    vector<string> tokens;
    const char s[]=" ";
    char *stop=strtok(raws, s);
    while(stop!=NULL){
        tokens.push_back(stop);
        stop=strtok(NULL,  "  ");
    }
    return tokens;
}

//function that chages the directory is beneath


string changeDir(const char* path = "") {
    const char* sign = "~";
    string home_path = getenv("HOME");

    if (!(strcmp(path, sign))) {
        prev_path = get_current_dir_name();
        chdir(home_path.c_str());
        return home_path;
    }

    const char* sign1 = "-";
    if (!(strcmp(path, sign1))) {
        if (!prev_path.empty()) {
            string temp = get_current_dir_name();
            chdir(prev_path.c_str());
            prev_path = temp;
            return prev_path;
        } else {
            cout << "No previous directory to return to." << endl;
            return "";
        }
    }

    const char* sign2 = "..";
    if (!(strcmp(path, sign2))) {
        if (!(strcmp(get_current_dir_name(), home_path.c_str()))) {
            return get_current_dir_name();
        } else {
            prev_path = get_current_dir_name();
            filesystem::path current_path = get_current_dir_name();
            filesystem::path parent_directory = current_path.parent_path();
            const char* path_for_cd = parent_directory.c_str();
            chdir(path_for_cd);
            return get_current_dir_name();
        }
    }

    string temp = get_current_dir_name();
    int directory_changed = chdir(path);
    if (directory_changed == 0) {
        prev_path = temp;
        string cwd1 = get_current_dir_name();
        return cwd1;
    } else {
        cout << "No such file or directory" << endl;
        return "";
    }
}

//this function handles the redirection

void outputRedirector(string output, string redirector, string output_file){
    cout<<redirector<<endl;
    if(redirector == ">>"){
    int fd=open(output_file.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
    int writes=write(fd, output.c_str(), output.size());
    close(fd);
    }
    else{
        int fd=open(output_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
        int writes=write(fd, output.c_str(), output.size());
        close(fd);
    }
}

//this below function  echos on terminal

string echo(const vector<string> &tokens){
    string echo_out="";
    int n=tokens.size();
    for(int i=1;i<n;i++){
        if((tokens[i]==">") || (tokens[i]==">>")){
            string redirector=tokens[i];
            string output_file=tokens[++i];
            outputRedirector(echo_out, redirector, output_file);
            return "";
        }
        else{
        echo_out+=tokens[i];
        echo_out+=" ";
        }
    }
    return echo_out;
}

//below function is implemented for cat command

void cat_the_file(string file_path, int mode, string output="", string redirector=""){
    int buffer_size=500;
    char buff[buffer_size];
    int fd1, n;
    int fd=open(file_path.c_str(), O_RDONLY, 0644);
    long long file_size=lseek(fd, (off_t)0, SEEK_END);
    long long data_left=file_size;
    lseek(fd, 0, SEEK_SET);
    if (fd < 0)
		{
		  cout << "Error in opening the file " << file_path << endl;
          exit(0);
        }

    if(mode){
        if(redirector==">"){
            fd1=open(output.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
        }
        else{
            fd1=open(output.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
        }
    if (fd1 < 0)
		{
		  cout << "Error in opening the file " << output << endl;
          exit(0);
		}
    while(data_left>0){
    int data_to_read = (data_left < buffer_size) ? data_left : buffer_size;
    n=read(fd, buff, data_to_read);
    // if (n == 0) {
    //     perror("Error in reading the file");
    //     cout << "Errno: " << errno << endl;
    //     exit(0);
    // }
    int writes=write(fd1, buff, n);
    // if (writes == 0) {
    //     perror("Error in reading the file");
    //     cout << "Errno: " << errno << endl;
    //     exit(0);
    // }
    data_left-=n;
    }
    close(fd);
    close(fd1);
    }
    else{
        while(data_left>0){
        int data_to_read = (data_left < buffer_size) ? data_left : buffer_size;
        n=read(fd, buff, data_to_read);
        // if (n == 0) {
        //     perror("Error in reading the file");
        //     cout << "Errno: " << errno << endl;
        //     exit(0);
        // }
        int writes=write(1, buff, n);
        data_left-=n;
        }
    } 
    close(fd);
    }


//this ls function normally prints files and directory name without any more details

int lsFunctionNormal(const char* path){
    struct stat file_s;
    if (stat(path, &file_s) == -1) {
        cout<< "Error getting in locating the file" <<endl;
    }
    if (!S_ISDIR(file_s.st_mode)) {
        cout << "The provided path is not a directory." << endl;
        return 1;
    }

    DIR *dir = opendir(path);
    if (dir == nullptr) {
        cout << "Error opening the directory" << endl;
        return 1;
    }

    dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
                cout << entry->d_name << "      ";
            }
        } 
    }
    cout<<endl;
    closedir(dir);
    return 0;
}

//this ls function normally prints files and directory name with -a as parameter

vector<string> lsFunctionA(const char* path){
    vector<string> files;
    struct stat file_s;
    if (stat(path, &file_s) == -1) {
        cout<< "Error getting in locating the file" <<endl;
    }
    if (!S_ISDIR(file_s.st_mode)) {
        cout << "The provided path is not a directory." << endl;
        exit;
    }

    DIR *dir = opendir(path);
    if (dir == nullptr) {
        cout << "Error opening the directory" << endl;
        exit;
    }
    dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
                files.push_back(entry->d_name);
            }
    }
    return files;
}

//this ls function normally prints files and directory name with -a as parameter

void printPermissions(mode_t mode) {
    if (S_ISDIR(mode)) {
        cout << "d";
    } else {
        cout << "-";
    }
    cout << ((mode & S_IRUSR) ? "r" : "-")
         << ((mode & S_IWUSR) ? "w" : "-")
         << ((mode & S_IXUSR) ? "x" : "-")
         << ((mode & S_IRGRP) ? "r" : "-")
         << ((mode & S_IWGRP) ? "w" : "-")
         << ((mode & S_IXGRP) ? "x" : "-")
         << ((mode & S_IROTH) ? "r" : "-")
         << ((mode & S_IWOTH) ? "w" : "-")
         << ((mode & S_IXOTH) ? "x" : "-");
}

vector<string> lsFunctionL(const char* path, int num){
    vector<string> files;
    struct stat file_s;
    if (stat(path, &file_s) == -1) {
        cout<< "Error getting in locating the file" <<endl;
    }
    if (!S_ISDIR(file_s.st_mode)) {
        cout << "The provided path is not a directory." << endl;
        exit;
    }

    DIR *dir = opendir(path);
    if (dir == nullptr) {
        cout << "Error opening the directory" << endl;
        exit;
    }
    dirent *entry;
    if(num==0){
    while((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            struct stat entry_s;
            string entryPath = string(path) + "/" + entry->d_name;
            if (stat(entryPath.c_str(), &entry_s) != -1) {
                printPermissions(entry_s.st_mode);
                cout << "   " << entry_s.st_nlink;
                string username = getenv("USER");
                // struct group *group_info = getgrgid(entry_s.st_gid);
                cout << "   " << username<< "   " << username;
                cout <<"        "<< entry_s.st_size;
                struct tm *time_info = localtime(&entry_s.st_mtime);
                char time_str[80];
                strftime(time_str, sizeof(time_str), "%b %d %H:%M", time_info);
                cout << "   " << time_str;
                cout << "   " << entry->d_name << endl;
            }
        }
    }
    }
    else{
        while ((entry = readdir(dir)) != nullptr) {
            struct stat entry_s;
            string entryPath = string(path) + "/" + entry->d_name;
            if (stat(entryPath.c_str(), &entry_s) != -1) {
                printPermissions(entry_s.st_mode);
                cout << "   " << entry_s.st_nlink;
                string username = getenv("USER");
                // struct group *group_info = getgrgid(entry_s.st_gid);
                cout << "   " << username<< "   " << username;
                cout <<"        "<< entry_s.st_size;
                struct tm *time_info = localtime(&entry_s.st_mtime);
                char time_str[80];
                strftime(time_str, sizeof(time_str), "%b %d %H:%M", time_info);
                cout << "   " << time_str;
                cout << "   " << entry->d_name << endl;
            }
        }
    }


    closedir(dir);
    return files;
}

//function that seraches for file in cwd

int search(const char* path, string filename){
    vector<string> files;
    struct stat file_s;
    if (stat(path, &file_s) == -1) {
        cout<< "Error getting in locating the file" <<endl;
    }
    if (!S_ISDIR(file_s.st_mode)) {
        cout << "The provided path is not a directory." << endl;
        exit;
    }

    DIR *dir = opendir(path);
    if (dir == nullptr) {
        cout << "Error opening the directory" << endl;
        exit;
    }
    dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
                if(entry->d_name==filename){
                    return 1;
                }
            }
    }
    return 0;
}

int main(){
    string cwd=getenv("HOME");
    chdir(cwd.c_str());
    string command;
    vector<string> tokens;
    while(true){
    displayOnScreen(cwd);
    tokens=tokeniser();
    if(tokens.size()!=0){
    command=tokens[0];
    if(command=="cd"){
            if(tokens.size()==1){
                cwd=cwd;
            }
            else if(tokens[1]!=""){
            cwd=changeDir(tokens[1].c_str());
            if(cwd==""){
                cwd=get_current_dir_name();
            }
            }
        }
        else if(command=="echo"){
            string write_out=echo(tokens);
            if(write_out!=""){
                cout<<write_out<<endl;
            }
        }
        else if(command=="pwd"){
            cout<<get_current_dir_name()<<endl;
        }
        else if(command=="ls"){
            if((tokens.size()==1) || (tokens[1]==".")){
            const char* path_for_ls=cwd.c_str();
            lsFunctionNormal(path_for_ls);
            }
            else if(tokens[1]=="~"){
            const char* path_for_ls=getenv("HOME");
            lsFunctionNormal(path_for_ls);
            }
            else if(tokens[1]==".."){
            filesystem::path current_path=cwd;
            filesystem::path parent_directory=current_path.parent_path();
            const char* path_for_ls=parent_directory.c_str();
            lsFunctionNormal(path_for_ls);
            }
            else if(tokens[1]=="-l" && tokens.size()==2){
            const char* path_for_ls=cwd.c_str();
            lsFunctionL(path_for_ls, 0);
            }
            else if(tokens[1]=="-a" && tokens.size()==2){
            const char* path_for_ls=cwd.c_str();
            vector<string> hidden_files=lsFunctionA(path_for_ls);
            for(auto i: hidden_files){
                cout<<i<<"      ";
            }
            cout<<endl;
            }
            else if((tokens[1]=="-a" && tokens[2]=="-l") || (tokens[1]=="-l" && tokens[2]=="-a")){
            vector<string> files_a;
            const char* path_for_ls=cwd.c_str();
            files_a=lsFunctionL(path_for_ls, 1);
            for(auto i:files_a){
                cout<<i;
            }
            }
            else if(((tokens[1]=="-la") || (tokens[1]=="-al")) && tokens.size()!=3){
            const char* path_for_ls=cwd.c_str();
            lsFunctionL(path_for_ls, 1);
            }
            else if(tokens.size()==2){
            string s=cwd+"/"+tokens[1];
            const char* path_for_ls=s.c_str();
            lsFunctionNormal(path_for_ls);
            }
            else if(((tokens[1]=="-la") || (tokens[1]=="-al")) && tokens.size()==3){
                string s=cwd+"/"+tokens[2];
                const char* path_for_ls=s.c_str();
                lsFunctionL(path_for_ls, 1);
            }
            else if((tokens[1]!="-l") || (tokens[1]!="-a")){
            string s=tokens[2];
            const char* path_for_ls=s.c_str();
            if(tokens[1]=="-l"){
            lsFunctionL(path_for_ls, 0);
            }
            else{
                vector<string> files_a;
                files_a=lsFunctionA(path_for_ls);
                for(auto i:files_a){
                cout<<i<<"      ";
            }
            cout<<endl;
            }
            }

        }
        else if(command=="cat"){
            if(tokens[1]=="<"){
                if(search(cwd.c_str(), tokens[2])){
                cat_the_file(tokens[2], 0);
                }
                else{
                    cout<<"No such file or directory"<<endl;
                }
            }
            else if(tokens.size()==2){
            if(search(cwd.c_str(), tokens[1])){
                cat_the_file(tokens[1], 0);
                }
                else{
                    cout<<"No such file or directory"<<endl;
                }
            }
            else if(tokens[2]==">" || tokens[2]==">>"){
                if(search(cwd.c_str(), tokens[1])){
                cat_the_file(tokens[1], 1, tokens[3], tokens[2]);
                }
                else{
                    cout<<"No such file or directory"<<endl;
                }
            }
        }
        else if(command=="search"){
            // std::string filePath = tokens[1];
            // fs::path pathObj(filePath);
            // std::string fileName = pathObj.filename();  // Get the file name
            // std::string directoryPath = pathObj.parent_path(); 
            // if(directoryPath==""){
                string filename=tokens[1];
                int flag=search(cwd.c_str(), filename);
                if(flag){
                    cout<<"True"<<endl;
                }
                else{
                    cout<<"False"<<endl;
                }
            // }
            // else{
            //     return search(directoryPath.c_str(), fileName);
            // }
        }
        else{
            cout<<command<<": "<<"command not found"<<endl;
        }
        }
    else{
        chdir(cwd.c_str());
    }
    }
}