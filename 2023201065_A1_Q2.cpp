#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include<string>
#include<sys/stat.h>

using namespace std;

void reverse_string(const string& input_file, int firstIndex, int secondIndex) {
    int fd = open(input_file.c_str(), O_RDONLY);
    if(fd<0){
    cout<<"File "<<input_file<< " cannot be opened.";
    return ;
    }
    long long int fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    vector<char> buff(fileSize);
    read(fd, buff.data(), fileSize);
    reverse(buff.begin(), buff.begin() + firstIndex);
    reverse(buff.begin() + secondIndex+1, buff.end());
    string name=input_file;
    string path="./Assignment1_2/2_";
    mkdir("./Assignment1_2", S_IRWXU);
    path+=name;
    int fd1=open(path.c_str(), O_WRONLY|O_CREAT, 0642);
    if(fd1<0){
    cout<<"File "<<path.c_str()<< " cannot be opened.";
    return ;
    }
    buff.erase(remove(buff.begin(), buff.end(), '\n'), buff.end());
    write(fd1, buff.data(), buff.size());
    close(fd);
    close(fd1);
    cout << "File stored in: " << path.c_str() << endl;
}

int main(int argc, char* argv[]) {
    string input_file = argv[1];
    int firstIndex = atoi(argv[2]);
    int secondIndex = atoi(argv[3]);
    reverse_string(input_file, firstIndex, secondIndex);
    return 0;
}

