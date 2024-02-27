#include <unistd.h>
#include <string.h>
#include <iostream>
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

string _removeFirstWords(string input, unsigned int n){
    string output = input;
    for (size_t i = 0; i < n and not output.empty(); i++)
    {
        output = _getTheRest(output);
    }
    return output;
}

string _get_nth_word(const string input, int n){
    return _getFirstWord(_removeFirstWords(input, n-1));
}

bool _command_is_two_numbers(string input){
    string first_word = _get_nth_word(input,1);
    string second_word = _get_nth_word(input,2);
    if (first_word.empty() || second_word.empty())
    {
        return false;
    }
    try
    {
        if (stoi(first_word) == 0 || stoi(second_word) == 0){return false;}
    }
    catch(const std::invalid_argument&)
    {
        return false;
    }
    return true;
}


//---------------------------------SMASH--------------------------------//

SmallShell::SmallShell() :  smash_pid(), prompt(),prev_path() {
    setCurrentPrompt(std::string());
}

SmallShell::~SmallShell() = default ;


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
        return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
      return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0) {
      return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0) {
      return new QuitCommand(cmd_line);
  }
  else {
      return new ExternalCommand(cmd_line);
  }
}

void SmallShell::executeCommand(const char *cmd_line) {
    delete_finished_jobs();
    Command *cmd = CreateCommand(cmd_line);
    if (!cmd){throw;}
    if (cmd->is_external())
    {
        pid_t new_pid = 0; //TODO
        jobsList.addJob(cmd, new_pid);
        //TODO: fork. run in background if "&" at end of command
        //status = fork()
        //setpgrp() //see HW instructions on this command
    }

      cmd->execute();

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::smash_print(const string input)
{
    cout << getCurrentPrompt() << PROMPT_SUFFIX << input << endl; //TODO: endl or not to endl?
}

void SmallShell::smash_error(const string input)
{
    cerr << ERROR_PROMPT << input << endl;
}

const string &SmallShell::getCurrentPrompt() const {
    return prompt;
}

void SmallShell::setCurrentPrompt(const string &new_prompt)
{
    if (new_prompt.empty())
        prompt = DEFAULT_PROMPT;
    else
        prompt = new_prompt;
}

string &SmallShell::getPrevPath() {
    return prev_path;
}

void SmallShell::printJobs() const{
    jobsList.printJobsList();
}

void SmallShell::killall()
{
    jobsList.killAllJobs();
}

pid_t SmallShell::getPidById(int Id)
{
    if (jobsList.getJobById(Id)){
        return jobsList.getJobById(Id)->get_pid();
    }
    else return 0;
}

void SmallShell::delete_finished_jobs() {
    jobsList.delete_finished_jobs();
}


//-----------------------------------------JOBS-------------------------------//

int JobsList::JobEntry::get_id() const {
    return id;
}

int JobsList::JobEntry::get_pid() const {
    return pid;
}

int JobsList::JobEntry::operator==(const JobEntry & other) const {
    return id == other.get_id();
}

std::string JobsList::JobEntry::get_command_name() {
    return cmd->get_name();
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
    return jobs[jobId];
}

void JobsList::delete_job_by_pid(pid_t pid){
    for (size_t i = 0; i < jobs.size(); i++)
    {
        if (jobs[i] && jobs[i]->get_pid() == pid)
        {
            jobs.erase(jobs.begin() + i); //deletes the i-th element from jobs
            return;
        }
    }
}

void JobsList::delete_finished_jobs() {
    pid_t child_pid;
    do
    {
        int status;
        child_pid = waitpid(-1, &status ,WNOHANG);
        if (child_pid > 0)
        {
            delete_job_by_pid(child_pid);
        }
        else break;

    } while (true); //while we deleted a child, so maybe there are more left.

//    for (int i = 0; i < MAX_JOBS-1; ++i) {
//        if (jobs[i]->is_deleted()){
//
//        }
//    }
}

JobsList::JobsList() : jobs(MAX_JOBS, nullptr) {}

void JobsList::addJob(Command *cmd, pid_t pid) {
    if (!cmd){throw(std::exception());} //TODO: exception syntax
    int new_id = get_new_id();
    jobs[new_id] = (new JobEntry(new_id, pid, cmd)); //TODO: need additional memory management?
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

void JobsList::killAllJobs()
{
    int jobs_num = 0;
    string to_print = "";
    for (size_t i = 0; i < jobs.size(); i++)
    {
        if (jobs[i])
        {
            jobs_num++;
            JobEntry* job = jobs[i];
            to_print += (job->get_pid()) + ": " + job->get_command_name() +"\n";
            kill(job->get_pid(),SIGKILL);
        }
    }
    cout << SmallShell::getInstance().getCurrentPrompt() << ": sending SIGKILL signal to " << jobs_num << "jobs \n" << endl;
    cout << to_print;
}

//---------------------------------COMMANDS---------------------------------//

std::string Command::get_name() const
{
    return cmd_line;
}

//--------------------------------BUILT-IN COMMANDS-----------------------//

void BuiltInCommand::smash_print(const string input)
{
    SmallShell::getInstance().smash_print(input);
}

void BuiltInCommand::smash_error(const string input)
{
    SmallShell::getInstance().smash_error(input);
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
void ShowPidCommand::execute() {
    smash_print(PID_IS + std::to_string(getpid()));
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
    SmallShell::getInstance().setCurrentPrompt(_get_nth_word(get_cmd_line(),2));
}

void JobsCommand::execute() {
    SmallShell::getInstance().printJobs();
}

void QuitCommand::execute()
{
    bool kill = ((_get_nth_word(get_cmd_line(),2)) == "kill");
    if (kill && _get_nth_word(get_cmd_line(),3).empty()) // the string is empty except first 2 words
    {
        SmallShell::getInstance().killall();
    }
    exit(0); //return 0
}


void KillCommand::execute()
{
    const string cmd = _getTheRest(get_cmd_line()); //the commmand except the word kill.
    if (not _command_is_two_numbers(cmd))
    {
        smash_error("kill: invalid arguments");
        return;
    }
    int signum = stoi(_getFirstWord(cmd));
    if (signum < MIN_SIGNUM || signum > MAX_SIGNUM) //TODO: is max signum correct?
    {
        smash_error("kill: invalid arguments");
        return;
    }

    int jobId = stoi(_getTheRest(cmd));
    pid_t target_pid = SmallShell::getInstance().getPidById(jobId);
    if (target_pid == 0)
    {
        smash_error(string("kill: job-id %d does not exist", jobId));
        return;
    }
    kill(signum, target_pid);
    cout << "signal number " << signum << " was sent to pid " << target_pid << endl;
}

//--------------------------------EXTERNAL COMMANDS------------------------//

void ExternalCommand::execute() {

}

