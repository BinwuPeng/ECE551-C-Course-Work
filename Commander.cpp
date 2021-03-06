#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include <regex>
#include <fcntl.h>
#include <sys/types.h>
#include "Commander.h"

extern char ** environ;

/*
Set the prompt string and store all the environment variable value
**/
Commander::Commander():prompt(),in_file(),out_file(),err_file(),num_child_processes(0),curr_command_index(0)
  {
    Set_Prompt_string();
    char ** envp = environ;
    while (*envp)
    {
      std::string curr_env(*envp);
      envp++;
      std::size_t equal_index = curr_env.find('=');
      var_map[curr_env.substr(0, equal_index)]=curr_env.substr(equal_index + 1);
    }
    
  }

bool Commander::is_valid_var(std::string var)
  {
    return std::regex_match(var, std::regex("^[_A-Za-z]\\w*$"));
  }
void Commander::set_var()
  {
    std::vector <std::string > cmd_vector =cmd_list[0];

    if(!is_valid_var(cmd_vector[1]))
      {
	std::cout<<"set vaible faild! Invalid variable number, can only set one car each time!!"<<std::endl;
	  return;
      }
    else if(cmd_vector.size()!=3)
      {
	std::cout<<"set vaible faild! Invalid variable value!"<<std::endl;
	return;
      }
    var_map[cmd_vector[1]]=cmd_vector[2];
    std::cout<<"set variable succeed!"<<std::endl;
  }

bool Commander::export_var()
  {
    std::vector <std::string > cmd_vector =cmd_list[0];
    //int putenv(char *string);
    if(cmd_vector.size()!=2)
      {
	std::cout<<"export variable failed, size is not 2"<<std::endl;
	return false;
      }
    std::map <std::string,std::string>::iterator target = var_map.find(cmd_vector[1]);
    if (target== var_map.end())
      {
	/* Not found */
	std::cout<<"export var failed, can't find the var["<<cmd_vector[1]<<"]"<<std::endl;
	return false;
      }
    else
      {
	/* Found, i->first is f, i->second is ++-- */
	std::string env_str= target->first+"="+target->second;
	std::cout<<"export: "<<env_str<<std::endl;
	char * cstr = new char [env_str.length()+1];
	std::strcpy (cstr, env_str.c_str());
	if(putenv(cstr)==0)
	  {
	    return true;
	  }
	else
	  {
	   std::cout<<"set vaible failed, system call putenv() failed!"<<std::endl;
	  return false;
	  }
      }
  }
  
void Commander::Set_Prompt_string( void )
  {
   const int PATH_MAX=128;
    char buff[PATH_MAX];
    getcwd( buff, PATH_MAX );
    std::string cwd( buff );
    prompt= "myShell:"+cwd+" $";
  }
void Commander::print_hint()
  {
    std::cout<<std::endl;
    std::cout<<prompt<<std::endl;
  }

std::string& Commander::rtrim(std::string& s, const char* t =" \t\n\r\f\v")
  {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
  }

  // trim from beginning of string (left)
std::string& Commander::ltrim(std::string& s, const char* t =" \t\n\r\f\v")
  {
    s.erase(0, s.find_first_not_of(t));
    return s;
  }
  

std::string Commander::remove_slash(std::string &argv)
  {
    while(argv.find('\\')!=std::string::npos)
      {
	argv=argv.erase(argv.find('\\'),1);
      }
    return argv;
  }

std::string Commander::replace_tab_with_space(std::string &argv)
  {
    while(argv.find('\t')!=std::string::npos)
      {
	argv=argv.replace(argv.find('\t'),1,1,' ');
      }
    return argv;
  }

  // trim from both ends of string (left & right)
std::string& Commander::trim(std::string& s, const char* t = " \t\n\r\f\v")
  {
    return ltrim(rtrim(s, t), t);
  }
  // test the exist and access of the cmd file
bool Commander::cmdExists(const std::string& file) {
    struct stat buf;
    return (stat(file.c_str(), &buf) == 0);
  }
std::string Commander::getcwd_string( void )
  {
    char buff[128];
    getcwd( buff, 128 );
    std::string cwd( buff );
    return cwd;
  }
/*
replace the $var with its stored value
*/
std::string  Commander::pre_parse_var(std::string cmd)
  {
    // std::cout<<"endter pre_parse_var():"<<std::endl;
    std::string s(cmd);
    std::string var_sign("$");
   
    std::string space(" \t\n");
    std::string::size_type n,n_plus;

    n = s.find_first_of(var_sign);
    while(n!=std::string::npos)
      {
	n_plus=s.find_first_of(space,n);

	if(n_plus!=std::string::npos)
	  {
	    std::string var=s.substr(n+1, n_plus-n-1);
	    //std::cout<<"var name: "<<s.substr(n+1, n_plus-n)<<std::endl;
	    if(var_map.find(var)!=var_map.end())
	      {
		s=s.replace(n, n_plus-n, var_map.find(var)->second);
	      }
	    
	    //std::cout<<"after:"<<s<<std::endl;
	    n = s.find_first_of(var_sign);
	  }
	else
	{
	  std::string var=s.substr(n+1, s.size()-n);
	  //std::cout<<"var name: "<<var<<std::endl;

	  if(var_map.find(var)!=var_map.end())
	    {
	      s=s.replace(n, n_plus-n, var_map.find(var)->second);
	    }
	  
	  break;
	}
      }
    return s;
  }
std::string Commander::parse_redirect_path(std::string cmd)
  {
    in_file.clear();
    out_file.clear();
    err_file.clear();
    std::string str(cmd);
    std::string::size_type n, n_delimiter, n_plus;

    std::string out_str(">");
    std::string in_str("<");
    std::string err_str("2>");
    std::string space(" \t\n\f\r\v");
    std::string delimter("|");

    //std::cout<<str<<std::endl;
    n=str.find_first_of(in_str);
    if (n!= std::string::npos)
      {
	n_plus=str.find_first_not_of(space, n+1);
	n_plus=str.find_first_of(space, n_plus);
	n_delimiter= str.find_first_of(delimter,n+1);
	n_plus=std::min(std::min(n_plus,n_delimiter),str.size());
	std::string infile= str.substr(n+1,n_plus-n-1);
	in_file=trim(infile);
	str=str.erase(n,n_plus-n);
      }
    n=str.find_first_of(out_str);
    if (n!= std::string::npos)
      {
	n_plus=str.find_first_not_of(space, n+1);
	n_plus=str.find_first_of(space, n_plus);
	n_delimiter= str.find_first_of(delimter,n+1);
	n_plus=std::min(std::min(n_plus,n_delimiter),str.size());
	std::string outfile= str.substr(n+1,n_plus-n-1);
	out_file=trim(outfile);
	str=str.erase(n,n_plus-n);
       }
    n=str.find(err_str);
    if (n!= std::string::npos)
      {
	n_plus=str.find_first_not_of(space, n+1);
	n_plus=str.find_first_of(space, n_plus);
	n_delimiter= str.find_first_of(delimter,n+1);
	n_plus=std::min(std::min(n_plus,n_delimiter),str.size());
	std::string errfile= str.substr(n+1,n_plus-n-1);
	err_file=trim(errfile);
	str=str.erase(n,n_plus-n);
      }
    return str;
  }

void Commander::parse_argv(std::string cmd, std::vector <std::string > &cmd_vector)
  {
    cmd_vector.clear();
    std::string::size_type size= cmd.size();
    // std::cout<<"cmd size:"<<size<<std::endl;
    std::string s= cmd;
    char space=' ';
    char b_slash='\\';
    s=trim(s);
    s=replace_tab_with_space(s);

    std::string::size_type n=s.find(space);
    while(n!=std::string::npos)
      {
	size=s.size();
	std::string argv="";
	if(s.at(n-1)==b_slash)
	  {
	    //std::cout<<"enter b_slash here\n";
	    while
	      (
	       (  (n+2)<size && s.at(n+1)==b_slash && s.at(n+2)==space  )
	       || (  (n+1)<size &&  s.at(n+1)!=space )
	       )
	      {
		if( (n+2)<size && s.at(n+1)==b_slash && s.at(n+2)==space   )
		  {
		    // std::cout<<"n=n+2\n";
		    n=n+2;
		  }
		else if(  (n+1)<size &&  s.at(n+1)!=space )
		  {
		    //std::cout<<"n++\n";
		    n++;
		  }
	      }//while
	  }//if
	//std::cout<<"n:"<<n<<std::endl;
	argv=s.substr(0,n+1);
	argv=rtrim(argv);
	s=s.substr(n+1);
	s=trim(s);
	//std::cout<<"argv:"<<argv<<std::endl;
	argv=remove_slash(argv);
	cmd_vector.push_back(argv);
	//std::cout<<"s:"<<s<<std::endl;
	n=s.find(space);
      }//while
    if(s.size()>0)
      {
	//std::cout<<"arg:"<<s<<std::endl;
	cmd_vector.push_back(s);
      }
  }
void Commander::clear_cmd()
{
  curr_command_index = 0;
  num_child_processes = 0;
  for(std::size_t i=0;i< cmd_list.size();i++ )
    {
      cmd_list[i].clear();
    }
  cmd_list.clear();
}

/*
replace the $var with its real values
store the input and out file name
And push the comand to cmd_list
Each command stored as a vector(with the rediret file removed)
*/
void Commander::parse_cmd(std::string cmd)
  {
    // std::cout<< "enter parse():"<<std::endl;
    clear_cmd();
      if(!(var_map.empty()))
      {
	cmd=pre_parse_var(cmd);
      }
    cmd=parse_redirect_path(cmd);
    std::string::size_type n;
    std::vector <std::string > one_comand;
    n=cmd.find_first_of("|");
    while(n!=std::string::npos)
      {
	std::string cmd_temp= cmd.substr(0, n);
	parse_argv(cmd_temp, one_comand);
	cmd_list.push_back(one_comand);
	cmd=cmd.substr(n+1);
	n=cmd.find_first_of("|");
      }
    parse_argv(cmd, one_comand);
    cmd_list.push_back(one_comand);
    return;
  }

std::string Commander::get_cmd_from_path (const std::string& cmd)
  {
    //check current directory
    if(cmdExists(cmd))
      {
	return cmd;
      }else if(cmdExists("./"+cmd))
      {
	return ("./"+cmd);
      }

    //esle check all the path to find the cmd file
    char* pPath;
    pPath = getenv ("PATH");
    if (pPath!=NULL)
      {
	//bool result=false;
	std::string path(pPath);
	//std::cout<<path<<std::endl;
	char de= ':';
	std::string::size_type n=path.find(de);
	while(n!=std::string::npos)
	  {
	    std::string one_path=path.substr(0,n);

	    if(cmdExists(one_path+"/"+cmd)){return (one_path+"/"+cmd);}
	    //std::cout<<one_path<<std::endl;
	    path=path.substr(n+1);
	    n=path.find(de);
	  }
	if(path.size()>0)
	  {
	    if(cmdExists(path+"/"+cmd)){return (path+"/"+cmd);}
	    //std::cout<<path<<std::endl;
	  }

      }
    return "";
  }

bool Commander::chpwd(std::string path)
  {
    //std::cout<<"chpwd()"<<std::endl;
    if (chdir(path.c_str())==0)
      {
	Set_Prompt_string();
	return true;
      }
    else {return false;}
  }
/* Build the argv from the command vector*/
char ** Commander::build_argv(std::vector <std::string > cmd_vec)
  {
    std::vector <std::string > cmd_vector=cmd_vec;
    unsigned int argc= cmd_vector.size();
    char ** argv= new char*[argc+1];
    std::string f_cmd=get_cmd_from_path(cmd_vector[0]);
    //set argv[0]
    char * cstr = new char [f_cmd.length()+1];
    std::strcpy (cstr, f_cmd.c_str());
    argv[0]=cstr;
    //set the other
    for(unsigned int i=1;i<argc;i++)
      {
	std::string one_cmd=cmd_vector[i];
	char * cstr = new char [one_cmd.length()+1];
	std::strcpy (cstr, one_cmd.c_str());
	argv[i]=cstr;
      }
    // set the last NULL
    argv[argc]=NULL;
    return argv;
  }

void Commander::check_status(int status)
  {
    if (WIFEXITED(status))
      { printf("Program exited with status %d\n", WEXITSTATUS(status));}
    else if (WIFSIGNALED(status))
      { printf("Program was killed by signal %d\n", WTERMSIG(status)); }
    else if (WIFSTOPPED(status))
      { printf("Program was stopped by signal %d\n", WSTOPSIG(status));}
    else if (WIFCONTINUED(status))
      { printf("Program was continued\n"); }
  }

void Commander::configCommandRedirect()
{
  if(curr_command_index==0)
    {
      if (!in_file.empty())
	{
	  close(0);
	  if (open(in_file.c_str(), O_RDONLY, 0) < 0) {
	    std::cerr << "cannot open the redirect input file: " << std::strerror(errno) << std::endl;
	    exit(1);
	  }
	}
    }
  if(curr_command_index== (cmd_list.size()-1))
    {
      if (!out_file.empty())
	{
	  close(1);
	  if (open(out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0)
	    {
	      std::cerr << "cannot open the redirect output file: " << std::strerror(errno) << std::endl;
	      exit(1);
	    }
	}
      if (!err_file.empty())
	{
	  close(2);
	  if (open(err_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0) {
	    std::cerr << "cannot open the redirect error file: " << std::strerror(errno) << std::endl;
	    exit(EXIT_FAILURE);
	  }
	}
    }
}

/*
 * link the first comand's stdout to the pipe[1]
 * link the last comand's stdin to pipe[n-2]
 * for the midle command connect both the stdin and stdout
*
*/
void Commander::configCommandPipe(bool redirect_input = true, bool redirect_output = true)
{
  std::size_t num_commands = cmd_list.size();
  std::size_t num_pipes = num_commands - 1;
  if(num_pipes<1) return;
  
  if (curr_command_index == 0)
    { // not the first command
      // close fd 0 for stdin, and redirect pipe R end to fd 0
      if (dup2(pipefd[1], 1) < 0) {
	std::cerr << "failed to redirect stdin: " << std::strerror(errno) << std::endl;
	exit(EXIT_FAILURE);
      }
    } else if (curr_command_index == (num_commands - 1) )
    {
      if (dup2(pipefd[2 * (curr_command_index - 1)], 0) < 0)
	{
	  std::cerr << "failed to redirect stdin: " << std::strerror(errno) << std::endl;
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      // close fd 0 for stdin, and redirect pipe R end to fd 0
      if (dup2(pipefd[2 * (curr_command_index - 1)], 0) < 0)
	{
	  std::cerr << "failed to redirect stdin: " << std::strerror(errno) << std::endl;
	  exit(EXIT_FAILURE);
	}
      // close fd 1 for stdout, and redirect pipe W end to fd 1
      if (dup2(pipefd[2 * curr_command_index + 1], 1) < 0) {
	std::cerr << "failed to redirect stdout: " << std::strerror(errno) << std::endl;
	exit(EXIT_FAILURE);
      }

    }
  // close all pipe fds, only use the new 0 and 1 for read and write
  for (std::size_t i = 0; i < 2 * num_pipes; i++)
    {
      if (close(pipefd[i]) < 0) {
	std::cerr << "failed to close pipes: " << std::strerror(errno) << std::endl;
	exit(EXIT_FAILURE);
      }
    }
}
void Commander::run_a_command(std::vector <std::string > cmd_vector)
{
  
  pid_t forkResult = fork();
  num_child_processes++;
  if (forkResult == -1)
    { // fork error, skip the command
      std::cerr << "failed to create a child process: " << std::strerror(errno) << std::endl;
      exit(1);
    }
  else if (forkResult == 0)
    {
      configCommandRedirect();
      configCommandPipe();
      char ** argv= build_argv(cmd_vector);
      execve(argv[0], argv, environ);
      std::cerr << "execve failed: " << std::strerror(errno) << std::endl;
      exit(1); // if execve returns, the child process fails, 
    }
  
}
void Commander::createPipes()
{
  int num_pipes = cmd_list.size() - 1;
  pipefd = new int[2 * num_pipes]; // 2 * number of pipe marks
  for (int i = 0; i < num_pipes; i++)
    {
      if (pipe(pipefd + 2 * i) < 0)
	{ // R end: 2 * i, W end: 2 * i + 1
	  std::cerr << "failed to create pipes: " << std::strerror(errno) << std::endl;
	  exit(1);
	  return;
	}
    }
}

void Commander::closePipes() {
  int num_pipes = cmd_list.size() - 1;
  for (int i = 0; i < 2 * num_pipes; i++)
    {
      if (close(pipefd[i]) < 0)
	{
	  std::cerr << "failed to close pipe " << i << ": " << std::strerror(errno) << std::endl;
	  exit(1);
	  return;
	}
    }
}

void Commander::run_comand_list()
{
  createPipes();
    for(curr_command_index=0;curr_command_index< cmd_list.size();curr_command_index++ )
    {
      run_a_command(cmd_list[curr_command_index]);
    }
  closePipes();
  waitForChildProcesses();
  delete[] pipefd;
}

void Commander::waitForChildProcesses() {
  int childStatus;
  for (std::size_t i = 0; i < num_child_processes; i++)
    {
      wait(&childStatus);
      if (i == num_child_processes - 1)
	{ // only print the exit status of the last "reportable" piped command
	  if (WIFEXITED(childStatus)) { // the child process is terminated normally
	    std::cout << "Program exited with status " << WEXITSTATUS(childStatus) << std::endl;
	  } else if (WIFSIGNALED(childStatus))
	    { // the child process is terminated due to receipt of a signal
	      std::cout << "Program was killed by signal " << WTERMSIG(childStatus) << std::endl;
	    }
	}
    }
}

/*
Can't have the pipe | at the end of comand and can't redict to file in the midle fo two pipe
*/
bool Commander::basic_check(std::string cmd)
{
  if(cmd.empty()) return false;
  std::string::size_type n1=cmd.find_first_of('|');
  std::string::size_type n2=cmd.find_last_of('|');
  std::string::size_type n3=cmd.find_last_of('<');
  std::string::size_type n4=cmd.find_first_of('>');
  
  if(n2== ( cmd.size()-1 ) )
    {std::cerr << "cannot have | at the end of input" << std::endl;return false;}

  if(n3!=std::string::npos && n2!=std::string::npos && n3 > n2)
    {std::cerr << "cannot redirect stdinput for the last command!" << std::endl;return false;}

  if(n1!=std::string::npos && n4!=std::string::npos && n4<n1)
    {std::cerr << "cannot redirect stdout or stderr  for the first command!" << std::endl;return false;}
  
  

  if(n1!=std::string::npos  && n2!=std::string::npos)
    {
      if(n1!=n2)
	{
	  cmd=cmd.substr(n1+1,n2-n1);
	  if(cmd.find_last_of('<')!=std::string::npos || cmd.find_last_of('>')!=std::string::npos )
	    {
	      std::cerr << "cannot redirect stdout, stdin, or stderr to file in the mid of pipe" << std::endl;return false;
	    }
	}
    }
  
  return true;
}

/*
run the cd ,set exprot and env directly
run other command in forked child process
*/

void Commander::run(std::string cmd)
  {

    //std::cout<< "enter run():"<<std::endl;
    
    if(!basic_check(cmd)){return;}
    parse_cmd(cmd);
    if(!(cmd_list.size()>0)) return;

    std::vector <std::string > cmd_vector=cmd_list[0];    
    //handle the CD command here
    if(cmd_vector[0].compare("cd")==0)
      {
	unsigned int argc= cmd_vector.size();
	//std::cout<<"enter cd command"<<std::endl;
	if(argc>2)
	  {  std::cout<<"cd only one path parameter!"<<std::endl; return; }
	else if(argc==2)
	  {
	    std::cout<<"chdir to:["<<cmd_vector[1]<<"]"<<std::endl;
	    if (chpwd(cmd_vector[1])) {
		std::cout<<"change dir succeed"<<std::endl;
	      }
	    else {
		std::cout<<"change dir failed"<<std::endl;
	      }
	  }
	else
	  {
	    char* pPath;
	    pPath = getenv ("HOME");
	    if (chpwd(pPath)) {
	      std::cout<<"change dir succeed"<<std::endl;
	    }
	    else {
	      std::cout<<"change dir failed"<<std::endl;
	    }
	    
	  }
	return;

      }
    if(cmd_vector[0].compare("env")==0)
      {
	system("env");
	return;
      }
    // set variable
    if(cmd_vector[0].compare("set")==0)
      {
	set_var();
	return;
      }
    // export variable
    if(cmd_vector[0].compare("export")==0)
      {
	if(export_var())
	  {
	    std::cout<<"export var succeed!"<<std::endl;
	  }
	else
	  {
	    std::cout<<"export var failed!"<<std::endl;
	  }
	return;
      }
    // else run normal command in forked child process
    for(size_t n=0; n<cmd_list.size(); n++)
      {
	std::vector <std::string > cmd_vector= cmd_list[n];
	std::string f_cmd=get_cmd_from_path(cmd_vector[0]);
	if(f_cmd.empty()) { std::cout<<"cmd: "<<cmd_vector[0]<<" not found!"<<std::endl;return;}
      }
    run_comand_list();

   
  }//end run()


Commander::~Commander()
  {
    //delete  cmd;
    //delete argv;
    //delete envp;
   
  }
