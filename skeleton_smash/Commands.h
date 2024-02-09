#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define DEFAULT_PROMPT std::string("smash")
#define PROMPT_SUFFIX std::string("> ")
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    std::string cmd_line;
public:
    Command(const char *cmd_line) : cmd_line(cmd_line) {}

    virtual ~Command() = default;

    virtual std::string execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {}

    std::string execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    std::string execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    std::string execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(const char *cmd_line, std::string *newPrompt) : BuiltInCommand(cmd_line) {};

    virtual ~ChangePromptCommand() {}

    std::string execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd);

    virtual ~ChangeDirCommand() {}

    std::string execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    std::string execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {}

    std::string execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    std::string execute() override;
};


class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {}

    std::string execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    std::string execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {}

    std::string execute() override;
};

class ChmodCommand : public BuiltInCommand {
public:
    ChmodCommand(const char *cmd_line);

    virtual ~ChmodCommand() {}

    std::string execute() override;
};


class SmallShell {
private:
    // TODO: Add your data members
    std::string current_prompt;
    SmallShell() : current_prompt(), smash_pid() {setCurrentPrompt(DEFAULT_PROMPT);}; // ctor
    pid_t smash_pid;

    void setCurrentPrompt(const std::string &new_prompt);
public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    std::string executeCommand(const char *cmd_line);
    const std::string &getCurrentPrompt() const;
};

#endif //SMASH_COMMAND_H_
