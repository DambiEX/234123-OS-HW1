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
#define WHITESPACE " \t\n\r\f\v"
#endif

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

SmallShell::SmallShell() : current_prompt(), smash_pid() {setCurrentPrompt(std::string());}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

Command* SmallShell::CreateCommand(const char* cmd_line) {

  string firstWord = _getTheRest(cmd_line);
  string theRest = _getTheRest(cmd_line);
/*
  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else */
  if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line, this);
  }
  else if (firstWord.compare("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line, this);}
  /*
  else if ...
  .....

  else {
    return new ExternalCommand(cmd_line);
  }
 */
  return nullptr;
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
    return current_prompt;
}

void SmallShell::setCurrentPrompt(const string &new_prompt) {
    if (new_prompt.empty())
        current_prompt = DEFAULT_PROMPT;
    else
        current_prompt = new_prompt + PROMPT_SUFFIX;
}

void ChangePromptCommand::execute() {
    smash->setCurrentPrompt(_getTheRest(get_cmd_line()));
}
