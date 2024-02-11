#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;
#define DEFAULT_PROMPT "smash> "
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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
void ShowPidCommand::execute() {
    cout << SmallShell::getInstance().getPrompt()<<" pid is " << getpid() << endl;
}

void ChangePrompt::execute() {
    size_t start = cmd_s.find_first_of(" \n");
    if (start != std::string::npos) {
        // This assumes there's a space after "chprompt"
        size_t end = cmd_s.find_first_of(" \n", start + 1);
        std::string newPrompt;
        if (end != std::string::npos) {
            newPrompt = cmd_s.substr(start + 1, end - start - 1);
        } else {
            newPrompt = cmd_s.substr(start + 1); // Take the rest if there's no second space
        }
        SmallShell::getInstance().setPrompt(newPrompt+"> ");

    } else {
        SmallShell::getInstance().setPrompt(DEFAULT_PROMPT);
    }
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

Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
      return new ChangePrompt(cmd_line, cmd_s);


  }
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
      cmd->execute();

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}