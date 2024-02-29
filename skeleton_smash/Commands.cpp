#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;
#define SMASH_BASH_PATH "/bin/bash"
#define SMASH_C_ARG "-c"

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const std::string WHITESPACE = " \n\r\t\f\v";
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

string _getFirstWord(string input){
    string input_s = _trim(string(input));
    return input_s.substr(0, input_s.find_first_of(" \n"));
}

string _getTheRest(string input) {
    //gets the entire string besides the first word, and returns the trimmed version of it
    string input_s = _trim(string(input));
    string firstWord = input_s.substr(0, input_s.find_first_of(" \n"));
    return _trim(input_s.substr(firstWord.length()));
}

SmallShell::SmallShell() :  smash_pid(), prompt(),prev_path(), jobs(), current_job(nullptr) {
    setCurrentPrompt(std::string());
}

SmallShell::~SmallShell() = default ;

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
void ShowPidCommand::execute() {
    cout << SmallShell::getInstance().getCurrentPrompt() << PID_IS << getpid() << endl;
}

void GetCurrDirCommand::execute() {
    char* cwd = getcwd(nullptr, 0); // Dynamically allocate buffer
    if (cwd != nullptr) {
        std::cout << cwd << std::endl; // Print the current working directory
        free(cwd); // Free the allocated buffer
    } else {
        perror("getcwd() error");
    }
}

void ChangeDirCommand::execute() {
    char *prm[COMMAND_ARGS_MAX_LENGTH];
    int number_of_words = _parseCommandLine(get_cmd_line().c_str(), prm);
    if (number_of_words > 2) { // too many arguments
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
    if (strcmp(prm[1], "-") == 0) { // if wants cd prev pwd
        if (SmallShell::getInstance().getPrevPath().empty()) {// no prev path
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        char buff[COMMAND_ARGS_MAX_LENGTH];
        if (getcwd(buff, COMMAND_ARGS_MAX_LENGTH) == nullptr) {
            cerr << "smash error: getcwd failed" << endl;
            return;
        }
        if (chdir(SmallShell::getInstance().getPrevPath().c_str()) == -1) {
            perror("smash error: chdir failed");
            return;
        }
        SmallShell::getInstance().getPrevPath() = buff;
    } else {
        char buff[COMMAND_ARGS_MAX_LENGTH];
        if (getcwd(buff, COMMAND_ARGS_MAX_LENGTH) == nullptr) {
            cerr << "smash error: getcwd failed" << endl;
            return;
        }
        if (chdir(prm[1]) == -1) {
            perror("smash error: chdir failed");
            return;
        }
        SmallShell::getInstance().getPrevPath() = buff;
    }
}




Command* SmallShell::CreateCommand(const char* cmd_line) {

  string firstWord = _getFirstWord(cmd_line);
//  string theRest = _getTheRest(cmd_line);

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else
  if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line, this);
  }
  else if (firstWord.compare("cd") == 0) {
      return new ChangeDirCommand(cmd_line);
  }

 else
    return new ExternalCommand(cmd_line);

  return nullptr;
}
shared_ptr<JobsList::JobEntry> JobsList::getJobByPid(const int& jobPid) const
{
    if(jobPid <= 0){
        return std::shared_ptr<JobsList::JobEntry>(nullptr);
    }
    for(const auto& job: this->jobs){
        if(job->getPid() == jobPid){
            return job;
        }
    }
    return std::shared_ptr<JobsList::JobEntry>(nullptr);
}
void ExternalCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    pid_t new_pid = fork();
    if (new_pid < 0){
        perror("smash error: fork failed");
        return;
    }
    else if (new_pid > 0){ // father
        if(get_cmd_line().compare("") != 0){
            smash.jobs.addJob(this, new_pid, false);
        }
        if(!isBgCommand())
        {
            smash.current_job = smash.jobs.getJobByPid(new_pid);
            waitpid(new_pid, NULL, WUNTRACED);
            smash.current_job = nullptr;
        }
    }
    else{ // son's code:
        setpgrp();
        char cmd_args[COMMAND_MAX_ARGS+1];
        strcpy(cmd_args, this->get_cmd_line().c_str());
        char bash_path[COMMAND_ARGS_MAX_LENGTH+1];
        strcpy(bash_path, SMASH_BASH_PATH);
        char c_arg[COMMAND_ARGS_MAX_LENGTH+1];
        strcpy(c_arg, SMASH_C_ARG);
        char *args[] = {bash_path, c_arg, cmd_args, NULL};
        if (execv(SMASH_BASH_PATH, args) == -1)
        {
            cerr << "smash error: execvp failed" << endl;
            return;
        }
    }
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:

      Command *cmd = CreateCommand(cmd_line);
      if (cmd){
          cmd->execute();
      }

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

const string &SmallShell::getCurrentPrompt() const {
    return prompt;
}

 string &SmallShell::getPrevPath() {
    return prev_path;
}

void SmallShell::setCurrentPrompt(const string &new_prompt) {
    if (new_prompt.empty())
        prompt = DEFAULT_PROMPT;
    else
        prompt = new_prompt + PROMPT_SUFFIX;
}

void ChangePromptCommand::execute() {
    //sets the second word in the input as the prompt. the first word is the command "chprompt" itself.
    smash->setCurrentPrompt(_getFirstWord(_getTheRest(get_cmd_line())));
}
