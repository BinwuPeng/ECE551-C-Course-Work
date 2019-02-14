#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


#include <fcntl.h>

#include <sys/stat.h>
#include <sys/wait.h>

extern char **environ;

int main(int argc, char **argv, char** env) {


  system("env");

  //printf("%s\n", "   ");
  //printf("%s\n", "   ");

  
  //char *argt[] = {"env", NULL};
  //execve("env", {"env",NULL}, environ);

}
