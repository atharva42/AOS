// #include "kernel/fcntl.h"
// #include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char** argv) {
  if (argc < 3 || argc > 3 || (argv[1][0] < '0' || argv[1][0] > '9') || (argv[2][0] < '0' || argv[2][0] > '9')) {
    fprintf(2, "Error: %s not int\n", argv[1]);
    exit(1);
  }
  set_priority(atoi(argv[1]),atoi(argv[2]));
  exit(0);
}