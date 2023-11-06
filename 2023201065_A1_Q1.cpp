#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
using namespace std;

int main (int argc, char *argv[])
{
  int n,n1, fd, fd1;
  fd = open (argv[1], O_RDONLY, 0400);
  if (fd < 0)
    {
      cout << "File " << argv[1] << " cannot be opened.";
      return 0;
    }
  else
    {
      string name = argv[1];
      string path = "./Assignment1_1/1_";
      mkdir ("./Assignment1_1", S_IRWXU);
      path += name;
      fd1 = open (path.c_str (), O_WRONLY | O_CREAT, 0642);
      if (fd1 < 0)
	{
	  cout << "File " << argv[2] << " cannot be opened.";
	  return 0;
	}
      else
	{
	  long long int file_size = lseek (fd, (off_t) 0, SEEK_END);
	  lseek (fd, 0, SEEK_SET);
	  int buffer_size = 1000000;
	  char buff[buffer_size];
	  int a = 0, b, temp, percent;
	  long long int data_left = file_size, count = 0;
	  while (data_left > 0)
	    {
	      int adjusted_data = (data_left < buffer_size) ? data_left : buffer_size;
	      n = read (fd, buff, adjusted_data);
	      if (n <= 0)
		{
		  cout << "Error in reading the file " << argv[1] << endl;
		  return 0;
		}
	      b = n - 1;
	      while (a < b)
		{
		  if (buff[a] == '\n')
		    {
		      a++;
		      continue;
		    }
		  if (buff[b] == '\n')
		    {
		      b--;
		      continue;
		    }
		  temp = buff[a];
		  buff[a] = buff[b];
		  buff[b] = temp;
		  a++;
		  b--;
		}
	      n1 = write (fd1, buff, adjusted_data);
	      if (n1 <= 0)
		{
		  cout << "Error in writing the file " << argv[2] << endl;
		  return 0;
		}
	      count += n;
	      data_left -= n;
	      percent = (count * 100 / file_size);
	      if (count < file_size)
		cout << percent<<"%" << "\t\r" << flush;
	      else
		cout << percent<<"%"<< flush<<endl;
	    }
	}
      cout << "Successfully written to " << path << endl;
    }
  close (fd);
  close (fd1);
  return 0;
}

