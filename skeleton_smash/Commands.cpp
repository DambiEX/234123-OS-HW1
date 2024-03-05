#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iomanip>
#include <algorithm>
#include "Commands.h"
#include <fstream>



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

string _getLastChar(const string input){
    return _trim(input.substr(input.find_last_not_of(" "), input.find_last_not_of(" ")));
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
        stoi(first_word);
        stoi(second_word);
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


int SmallShell::get_num_jobs() const
{

    // Calculate the number of null elements (i.e., elements equal to nullptr)
    int NullCount = std::count(jobsList.jobs.begin(), jobsList.jobs.end(), nullptr);

    // Calculate the total number of elements in the vector
    int totalElements = jobsList.jobs.size();

    // Count the number of non-null elements
    int notNullCount = totalElements - NullCount;
    return notNullCount;
}





std::shared_ptr<Command> SmallShell::CreateCommand(std::string cmd_line) {

    string firstWord = _getFirstWord(cmd_line);
  if (firstWord.compare("chprompt") == 0) {
    return std::shared_ptr<Command>(new ChangePromptCommand(cmd_line));
  }
  else
  if (firstWord.compare("showpid") == 0) {
    return std::shared_ptr<Command>(new ShowPidCommand(cmd_line));
  }
  else if (firstWord.compare("pwd") == 0) {
    return std::shared_ptr<Command>(new GetCurrDirCommand(cmd_line));
  }
  else if (firstWord.compare("cd") == 0) {
      return std::shared_ptr<Command>(new ChangeDirCommand(cmd_line));
  }
  else if (firstWord.compare("jobs") == 0) {
      return std::shared_ptr<Command>(new JobsCommand(cmd_line));
  }
  else if (firstWord.compare("fg") == 0) {
      return std::shared_ptr<Command>(new ForegroundCommand(cmd_line));
  }
  else if (firstWord.compare("quit") == 0) {
      return std::shared_ptr<Command>(new QuitCommand(cmd_line));
  }
  else if (firstWord.compare("kill") == 0) {
      return std::shared_ptr<Command>(new KillCommand(cmd_line));
  }
    else if (firstWord.compare("chmod") == 0) {
      return std::shared_ptr<Command>(new ChmodCommand(cmd_line));
  }
  else {
      return std::shared_ptr<Command>(new ExternalCommand(cmd_line));
  }
}

void SmallShell::executeCommand(std::string cmd_line) {
    delete_finished_jobs();

    int cout_fd = setIO(cmd_line);
    std::string cmd_line_edit = cmd_line;
    if (cout_fd == ERROR_FD) return;
    else if (cout_fd >= 0){
    cmd_line_edit = _trim(cmd_line.substr(0, cmd_line.find(">")));
    }
    std::shared_ptr<Command> cmd = CreateCommand(cmd_line_edit);
    if (!cmd){throw;}
    cmd->execute();
    
    defaultIO(cout_fd);
}

void SmallShell::smash_print(const string input)
{
    std::cout << getCurrentPrompt() << PROMPT_SUFFIX << input << endl; //TODO: endl or not to endl?
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

void SmallShell::addJob(std::string cmd, pid_t pid)
{
    jobsList.addJob(cmd, pid);
}

void SmallShell::deleteJob(pid_t pid)
{
    jobsList.delete_job_by_pid(pid);
}

void SmallShell::printJobs() const{
    jobsList.printJobsList();
}

void SmallShell::killall()
{
    jobsList.killAllJobs();
}

std::shared_ptr<JobsList::JobEntry> SmallShell::getJobById(int Id)
{
    return jobsList.getJobById(Id);
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

int get_redirection_type(std::string cmd_line,__SIZE_TYPE__ pos, bool pipe = false)
{
    if (pos == std::string::npos){
        return 0;
    }
    char special_char = '>';
    if (pipe)
    {
        special_char = '&';
    }
    if (cmd_line[pos+1] == special_char) //>> append
    {
        return APPEND;
    }
    else return OVERWRITE;
}

int SmallShell::setIO(std::string cmd_line)
{
    int piping = not PIPE;
    __SIZE_TYPE__ pos = cmd_line.find(">");
    if (not pos)
    {
        pos = cmd_line.find("|");
        if (pos)
        {
            piping = PIPE;
        }
    }
    int redirection_type = get_redirection_type(cmd_line, pos, piping);
    if (not redirection_type)
    {
        return -1;
    }
    
    if (piping) return setPipe(redirection_type, cmd_line);

    string output_path = _trim(cmd_line.substr(pos+redirection_type));
    int old_cout = dup(STDOUT_FILENO);
    int fd;
    if (redirection_type == APPEND){ //>> append
        fd = open(output_path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0755);
    }
    else{ //> overwrite
        fd = open(output_path.c_str(), O_CREAT | O_WRONLY, 0755);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return old_cout;
}

void SmallShell::defaultIO(int old_cout){
    if(old_cout >= 0)
    {
        close(STDOUT_FILENO);
        dup2(old_cout, STDOUT_FILENO);
        close(old_cout);
    }
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
    return cmd;
}

std::shared_ptr<JobsList::JobEntry> JobsList::getJobById(int jobId)
{
    return jobs[jobId];
}

void JobsList::delete_job_by_pid(pid_t pid){
    for (size_t i = 0; i < jobs.size(); i++)
    {
        if (jobs[i] && jobs[i]->get_pid() == pid)
        {
            // jobs.erase(jobs.begin() + i); //deletes the i-th element from jobs
            jobs[i] = nullptr;
            return;
        }
    }
}

void JobsList::delete_job_by_id(int jobId)
{
    jobs.erase(jobs.begin() + jobId);
}

void JobsList::delete_finished_jobs() {
    pid_t child_pid;
    int status;
    do
    {
        child_pid = waitpid(-1, &status, WNOHANG);
        if (child_pid > 0)
        {
            delete_job_by_pid(child_pid);
        }
    } while (child_pid > 0); //while we deleted a child, so maybe there are more left.
}

JobsList::JobsList() : jobs(MAX_JOBS, nullptr) {}

void JobsList::addJob(std::string cmd, pid_t pid) {
    if (cmd.empty()){throw(std::exception());} //TODO: exception syntax
    int new_id = get_new_id();
    jobs[new_id] = (std::shared_ptr<JobEntry>(new JobEntry(new_id, pid, cmd)));
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
        if (jobs[i])
        {
            std::cout << "[" << jobs[i]->get_id() << "] " << jobs[i]->get_command_name() << endl;
        }
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
            to_print += (std::to_string(jobs[i]->get_pid())) + string(": ") + jobs[i]->get_command_name() + string("\n");
            kill(jobs[i]->get_pid(),SIGKILL);
        }
    }
    std::cout << SmallShell::getInstance().getCurrentPrompt() << ": sending SIGKILL signal to " << jobs_num << " jobs" << endl;
    std::cout << to_print;
}

//---------------------------------COMMANDS---------------------------------//

bool Command::run_in_foreground()
{
    return (_getLastChar(get_cmd_line()) != (string("&")));
}

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

void ChangePromptCommand::execute() {
    //sets the second word in the input as the prompt. the first word is the command "chprompt" itself.
    SmallShell::getInstance().setCurrentPrompt(_get_nth_word(get_cmd_line(),2));
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

void JobsCommand::execute() {
    SmallShell::getInstance().printJobs();
}

void ForegroundCommand::execute()
{
    if (_get_nth_word(get_cmd_line(),2).empty()) //no second argument
    {
        if (SmallShell::getInstance().get_num_jobs() == 0)
        {
            smash_error("fg: jobs list is empty");
        }
        else
        {
            smash_error("fg: invalid arguments");
        }
        return;
    }
    if (not _get_nth_word(get_cmd_line(), 3).empty()) // string has more than 2 words
    {
        smash_error("fg: invalid arguments");
        return;
    }

    //string is exactly 2 words long, and the first word is "fg"
    int job_id;
    try
    {
        job_id = stoi(_get_nth_word(get_cmd_line(),2));
    }
    catch(const std::invalid_argument&)
    {
        smash_error("fg: invalid arguments");
        return;
    }
    std::shared_ptr<JobsList::JobEntry> job = SmallShell::getInstance().getJobById(job_id);
    if (job == nullptr)
    {
        smash_error("job-id " + std::to_string(job_id) + " does not exist");
    }
    else
    {
        std::cout << job->get_command_name() << job->get_pid() << endl;
        waitpid(job->get_pid(),nullptr,0);
        SmallShell::getInstance().deleteJob(job->get_pid());
    }
}

void QuitCommand::execute()
{
    bool kill = ((_get_nth_word(get_cmd_line(),2)) == string("kill"));
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
    int signum = -stoi(_getFirstWord(cmd));
    if (signum < MIN_SIGNUM || signum > MAX_SIGNUM) //TODO: is max signum correct?
    {
        smash_error("kill: invalid arguments");
        return;
    }

    int jobId = stoi(_getTheRest(cmd));
    pid_t target_pid = SmallShell::getInstance().getPidById(jobId);
    if (target_pid == 0)
    {
        smash_error("kill: job-id " + std::to_string(jobId) + " does not exist");
        return;
    }
    kill(target_pid, signum);
    // SmallShell::getInstance().deleteJob(jobId);
    std::cout << "signal number " << signum << " was sent to pid " << target_pid << endl;
}

//--------------------------------EXTERNAL COMMANDS------------------------//

int SmallShell::setPipe(int redirection_type,  std::string cmd_line)
{
    if (redirection_type == 0) return -1;
    int pos = cmd_line.find("|");
    int my_pipe[2];
    int pipe_worked = pipe(my_pipe);
    if (pipe_worked)    
    {
        pid_t new_pid = fork();
        if (new_pid < 0){
            perror("smash error: fork failed");
            return -1;
        }
            else if (new_pid > 0){ // parent
            int old_cout = dup(STDIN_FILENO);
            close(my_pipe[1]); //close write
            dup2(my_pipe[0],STDIN_FILENO); //set stdin to be pipe read
            close(my_pipe[0]);
            std::shared_ptr<Command> in_command = SmallShell::getInstance().CreateCommand(_trim(cmd_line.substr(pos + redirection_type)));
            in_command->execute();
            close(STDIN_FILENO);
            dup2(old_cout, STDIN_FILENO);
            close(old_cout);
            return ERROR_FD;      
        }
        else
        {
            setpgrp();
            close(my_pipe[0]); //close read
            dup2(my_pipe[1], redirection_type); //set stdout/err to be pipe write
            close(my_pipe[1]);
            std::shared_ptr<Command> in_command = SmallShell::getInstance().CreateCommand(_trim(cmd_line.substr(0,pos)));
            in_command->execute();
            exit(0);
        }
    }
    else perror("pipe creation failed"); //TODO: error code
    return -1;     
}

void ExternalCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();    
    pid_t new_pid = fork();
    if (new_pid < 0){
        perror("smash error: fork failed");
        return;
    }
    else if (new_pid > 0){ // parent
        if(not get_cmd_line().empty()){
            smash.addJob(get_name(), new_pid);
            if (run_in_foreground())
            {
                waitpid(new_pid, NULL, 0);
                smash.deleteJob(new_pid);
            }
        }
    }
    else{ // child's code:
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

bool isValidOctal(const std::string& str) {
    if (str.size() != 3)
    {
        return false;
    }
    
    for (char c : str) {
        if (c < '0' || c > '7')
            return false;
    }
    return true;
}

void ChmodCommand::execute()
{
    if (not _removeFirstWords(get_cmd_line(),3).empty())
    {
        smash_error("chmod: invalid aruments");
        return;
    }

    // Extract new mode from command line arguments
    int new_mode;
    string second_word = _get_nth_word(get_cmd_line(),2);
    if (! isValidOctal(second_word))
    {
        smash_error("chmod: invalid aruments");
        return;    
    }
    
    if (second_word.empty())
    {
        smash_error("chmod: invalid aruments");
        return;    
    }
    try
    {
        new_mode = stoi(second_word, nullptr, OCTAL);
    }
    catch(const std::invalid_argument&)
    {
        smash_error("chmod: invalid aruments");
        return;
    }
    const char* path = _get_nth_word(get_cmd_line(),3).c_str();

    // Change file mode
    cout << "new mode: " << new_mode << " path: " << path << endl;
    if (chmod(path, new_mode) == 0) {
        std::cout << "File mode changed successfully." << std::endl;
    } else {
        std::cerr << "Failed to change file mode." << std::endl;
    }
}
