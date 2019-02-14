#include "Commander.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
/*
********************************************
usage:
[1]
./myShell   
Will run in inractive way
input "exit" to quit the Myshell

[2]
./myShell file1 file2 file2  
will run the comand line in those files.
The command in those files must be one line a compeleted command.
*********************************** 
*/

int  main(int argc, char *argv[], char ** envp)
{
  std::string cmd;
  Commander cmd_runner;

  if(argc==1)  // run in interactive way
    {
      cmd_runner.print_hint();
      while(std::getline (std::cin,cmd))
	{
	  if(cmd.compare("exit")==0)
	    {
	      exit(EXIT_SUCCESS);
	    }
	  else
	    {
	      cmd_runner.run(cmd);
	    }

	  cmd_runner.print_hint();

	}
      exit(EXIT_SUCCESS);
    }
  else
    {// read cmd from file and execute line by line
      for (int i = 1; i < argc; i++)
	{
	  std::ifstream input;
	  input.open(argv[i]);
	  if (!input.good())
	  {
	    std::cerr << "Error in opening file." << std::endl;
	    exit(EXIT_FAILURE);
	  }
	  else
	    {
	      std:: string one_line;
	      while(getline(input,one_line))
		{
		  cmd_runner.run(one_line);
		}
	      input.close();
	    }
      	}
      printf("%s\n", "finished all the command from the test case file");
      exit(EXIT_SUCCESS);
    }
}
