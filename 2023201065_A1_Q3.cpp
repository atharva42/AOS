#include<iostream>
#include<filesystem>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>
#include<algorithm>
using namespace std;

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    struct stat file_s;
    if (stat(argv[1], &file_s) == -1) {
        cout<< "Error getting in locating the file" <<endl;
        return 1;
    }
    const char *dir_s = argv[3];
    int d=0;
    if (fs::exists(dir_s) && fs::is_directory(dir_s)) {
        std::cout << "Directory is created. "<<"Yes" <<endl;
    } else {
        std::cout <<"Directory is created. "<<"No" <<endl;
    }
    
    //here the content in the new file are the reverse of the old file are checked
    
    
    int fd, fd1, file_size,n;
    bool flag1=true;
    int flag2;
    fd = open(argv[1], O_RDONLY, 0400);
    if (fd < 0) {
        cout << "File " << argv[1] << " cannot be opened.";
        return 0;
    } else {
        fd1 = open(argv[2], O_RDONLY, 0400);
        if (fd1 < 0) {
            cout << "File " << argv[2] << " cannot be opened.";
            return 0;
        } else {
            long long file_size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            int buffer_size = 1000000;
	    char buff[buffer_size];
            char buff2[buffer_size];
            long long int data_left = file_size;
            while(data_left>0){
            int adjusted_data = (data_left < buffer_size) ? data_left : buffer_size;
	        n = read (fd, buff, adjusted_data);
            if (n <= 0) {
                cout << "Error in reading the file " << argv[1] << endl;
                return 0;
            }
            int n1 = read(fd1, buff2, adjusted_data);
            if (n1 <=0) {
                cout << "Error in reading the file " << argv[2] << endl;
                return 0;
            }
            int buff_len = n;
            int buff2_len = n;
            int new_length_1 = 0;
            for (int i = 0; i < buff_len; i++) {
                if (buff[i] != '\n') {
                    buff[new_length_1++] = buff[i];
                }
            }
            int new_length_2 = 0;
            for (int i = 0; i < buff2_len; i++) {
                if (buff2[i] != '\n') {
                    buff2[new_length_2++] = buff2[i];
                }
            }
            int a = 0, b = n - 1;
            while (a < b) {
                char temp = buff[a];
                buff[a] = buff[b];
                buff[b] = temp;
                a++;
                b--;
            }            
            for(int x=0; x<adjusted_data; x++){
             	if(buff[x]!=buff2[x]){
             	flag1=false;
             	break;
             	}
             }
            flag2=(new_length_1 == new_length_2);
            data_left -= adjusted_data;
        }
        if (flag1 && flag2) {
                cout << "Whether file contents are reversed in newfile: Yes" << endl;
            } else {
                cout << "Whether file contents are reversed in newfile: No" << endl;
            }
        }
        close(fd);
        close(fd1);
    }
    
    
    // Here is the code to check for file permission 
    
    
    mode_t permissions = file_s.st_mode; 
            if (permissions & S_IRUSR)
                cout << "User has read permissions on newfile:"<<" Yes"<<endl;
            else
                cout << "User has read permissions on newfile"<<" NO"<<endl;
            if (permissions & S_IWUSR)
                cout << "User has write permissions on newfile:"<<" Yes"<<endl;
            else
                cout << "User has write permissions on newfile"<<" NO"<<endl;
            if (permissions & S_IXUSR)
                cout << "User has execute permissions on newfile:"<<" Yes"<<endl;
            else
                cout << "User has execute permissions on newfile:"<<" No"<<endl;
            if (permissions & S_IRGRP)
                cout << "Group has read permissions on newfile:"<<" Yes"<<endl;
            else
            	cout << "Group has read permissions on newfile:"<<" No"<<endl;
            if (permissions & S_IWGRP)
                cout << "Group has write permissions on newfile:"<<" Yes"<<endl;
            else
            	cout << "Group has write permissions on newfile:"<<" No"<<endl;
            if (permissions & S_IXGRP)
                cout << "Group has execute permissions on newfile:"<<" Yes"<<endl;
            else
            	cout << "Group has execute permissions on newfile:"<<" No"<<endl;
            if (permissions & S_IROTH)
                cout << "Others have read permissions on newfile:"<<" Yes"<<endl;
            else
            	cout << "Others have read permissions on newfile:"<<" No"<<endl;
            if (permissions & S_IWOTH)
                cout << "Others have write permissions on newfile:"<<" Yes"<<endl;
            else
            	cout << "Others have write permissions on newfile:"<<" No"<<endl;
            if (permissions & S_IXOTH)
                cout << "Others have execute permissions on newfile:"<<" Yes"<<endl;
            else
            	cout << "Others have execute permissions on newfile:"<<" No"<<endl;
            cout <<endl;
            
 //directory permissions will be checked from here
 	
            
     struct stat dirStat;
    if (stat(dir_s, &dirStat) == -1) {
        std::cerr << "Error getting directory information." << std::endl;
        return 1;
    }
     mode_t dirPermissions = dirStat.st_mode;
     if (dirPermissions & S_IRUSR)
                cout << "User has read permissions on  directory:"<<" Yes"<<endl;
            else
                cout << "User has read permissions on directory:"<<" NO"<<endl;
            if (dirPermissions & S_IWUSR)
                cout << "User has write permissions on directory:"<<" Yes"<<endl;
            else
                cout << "User has write permissions on directory:"<<" NO"<<endl;
            if (dirPermissions & S_IXUSR)
                cout << "User has execute permissions on  directory:"<<" Yes"<<endl;
            else
                cout << "User has execute permissions on  directory:"<<" No"<<endl;
            if (dirPermissions & S_IRGRP)
                cout << "Group has read permissions on  directory:"<<" Yes"<<endl;
            else
            	cout << "Group has read permissions on  directory:"<<" No"<<endl;
            if (dirPermissions & S_IWGRP)
                cout << "Group has write permissions on  directory:"<<" Yes"<<endl;
            else
            	cout << "Group has write permissions on  directory:"<<" No"<<endl;
            if (dirPermissions & S_IXGRP)
                cout << "Group has execute permissions on  directory:"<<" Yes"<<endl;
            else
            	cout << "Group has execute permissions on  directory:"<<" No"<<endl;
            if (dirPermissions & S_IROTH)
                cout << "Others have read permissions on  directory:"<<" Yes"<<endl;
            else
            	cout << "Others have read permissions on  directory:"<<" No"<<endl;
            if (dirPermissions & S_IWOTH)
                cout << "Others have write permissions on  directory:"<<" Yes"<<endl;
            else
            	cout << "Others have write permissions on  directory:"<<" No"<<endl;
            if (dirPermissions & S_IXOTH)
                cout << "Others have execute permissions on  directory:"<<" Yes"<<endl;
            else
            	cout << "Others have execute permissions on  directory:"<<" No"<<endl;
            cout <<endl;

    return 0;
}

