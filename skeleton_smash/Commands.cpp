#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

//----------------------------------------HELPER FUNCTIONS-----------------------//

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


//---------------------------------SMASH--------------------------------//

SmallShell::SmallShell() :  smash_pid(), prompt(), curr_path(""), path_history("") {
    setCurrentPrompt(std::string());
}

SmallShell::~SmallShell() {
// TODO: add your implementation
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
        return new ChangePromptCommand(cmd_line, this);}
        /*
        else if ...
        .....*/
    else {
        return new ExternalCommand(cmd_line);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    delete_finished_jobs();
    Command *cmd = CreateCommand(cmd_line);
    if (!cmd){throw;}
    if (cmd->is_external())
    {
        jobsList.addJob(cmd);
        //TODO: fork. run in background if "&" at end of command
        //status = fork()
        //setpgrp() //see HW instructions on this command
    }

      cmd->execute();

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

const string &SmallShell::getCurrentPrompt() const {
    return prompt;
}

void SmallShell::setCurrentPrompt(const string &new_prompt) {
    if (new_prompt.empty())
        prompt = DEFAULT_PROMPT;
    else
        prompt = new_prompt + PROMPT_SUFFIX;
}

void SmallShell::printJobs() const{
    jobsList.printJobsList();
}

void SmallShell::delete_finished_jobs() {
    jobsList.delete_finished_jobs();
}


//-----------------------------------------JOBS-------------------------------//

int JobsList::JobEntry::get_id() const {
    return id;
}

int JobsList::JobEntry::operator==(const JobEntry & other) const {
    return id == other.get_id();
}

std::string JobsList::JobEntry::get_command_name() {
    return cmd->get_name();
}

bool JobsList::JobEntry::is_deleted() {
    return waitpid();
}

void JobsList::delete_finished_jobs() {
    pid_t child;
    do
    {
        child = waitpid()
    } while ();

//    for (int i = 0; i < MAX_JOBS-1; ++i) {
//        if (jobs[i]->is_deleted()){
//
//        }
//    }
}

JobsList::JobsList() : jobs(MAX_JOBS, nullptr) {}

void JobsList::addJob(Command *cmd, bool isStopped) {
    if (!cmd){throw(std::exception());} //TODO: exception syntax
    int new_id = get_new_id();
    jobs[new_id] = (new JobEntry(new_id, cmd)); //TODO: need additional memory management?
}

int JobsList::get_new_id() {
    for (int i = 1; i < MAX_JOBS-1; ++i) {
        if (jobs[i] == nullptr){
            return i;
        }
    }
    throw;
}

void JobsList::printJobsList() const{
    for (unsigned int i=0; i<jobs.size(); i++){
        std::cout << "[" << i << "] " << jobs[i]->get_command_name() << endl;
    }
}

//---------------------------------COMMANDS---------------------------------//

std::string Command::get_name() const {
    return cmd_line;
}


//--------------------------------BUILT-IN COMMANDS-----------------------//

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
void ShowPidCommand::execute() {
    cout << SmallShell::getInstance().getCurrentPrompt() << PID_IS << getpid() << endl;
}

void GetCurrDirCommand::execute() {
    char* cwd = getcwd(NULL, 0); // Dynamically allocate buffer
    if (cwd != nullptr) {
        std::cout << cwd << std::endl; // Print the current working directory
        free(cwd); // Free the allocated buffer
    } else {
        perror("getcwd() error");
    }
}

void ChangePromptCommand::execute() {
    //sets the second word in the input as the prompt. the first word is the command "chprompt" itself.
    smash->setCurrentPrompt(_getFirstWord(_getTheRest(get_cmd_line())));
}

void JobsCommand::execute() {
    smash->printJobs();
}

//--------------------------------EXTERNAL COMMANDS------------------------//

void ExternalCommand::execute() {

}
