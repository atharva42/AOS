// #include "kernel/fcntl.h"
// #include "kernel/param.h"
#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char** argv) {
  if (argc < 2 || argc > 2 || (argv[1][0] < '0' || argv[1][0] > '9') || (argv[2][0] < '0' || argv[2][0] > '9')) {
    fprintf(2, "Error: %s not int\n", argv[1]);
    exit(1);
  }
  // sigalarm(atoi(argv[1]), &(uint64) atoi(argv[2]));
  sigalarm(atoi(argv[1]), (void(*)()) (uint64)atoi(argv[2]));
  exit(0);
}