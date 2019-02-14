#ifndef __COMMANDER_H__
#define __COMMANDER_H__

#include <string>
#include <map>
#include <vector>

class Commander
{
 private:
  std::string prompt;
  std::string in_file;
  std::string out_file;
  std::string err_file;
  std::size_t num_child_processes;
  std::size_t curr_command_index;
  
  int * pipefd; // an array of pipe fd used to run piped commands
  std::map <std::string,std::string>  var_map;
  std::vector < std::vector <std::string > > cmd_list;
  
  bool is_valid_var(std::string var);
  void set_var();
  bool export_var();
  void createPipes();
  void closePipes();
  void Set_Prompt_string();
  std::string& rtrim(std::string& s, const char* t );
  std::string& ltrim(std::string& s, const char* t );
  std::string remove_slash(std::string &argv);
  std::string replace_tab_with_space(std::string &argv);
  std::string& trim(std::string& s, const char* t );
  bool cmdExists(const std::string& file);
  std::string getcwd_string( void );
  std::string  pre_parse_var(std::string cmd);
  void clear_cmd();
  std::string parse_redirect_path(std::string cmd);
  void parse_argv(std::string cmd, std::vector <std::string > &cmd_vector);
  void parse_cmd(std::string cmd);
  std::string get_cmd_from_path (const std::string& cmd);
  bool chpwd(std::string path);
  char ** build_argv(std::vector <std::string > cmd_vec);
  void check_status(int status);
  void run_comand_list();
  void configCommandRedirect();
  void configCommandPipe(bool redirect_input , bool redirect_output);
  void waitForChildProcesses();
  bool basic_check(std::string cmd);
  void run_a_command(std::vector <std::string > cmd_vector);

public:
  Commander();
  void print_hint();
  void run(std::string cmd);
   ~Commander();
};

#endif
