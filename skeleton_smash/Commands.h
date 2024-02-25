#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#define DEFAULT_PROMPT std::string("smash> ")
#define PROMPT_SUFFIX std::string("> ")
#define PID_IS std::string(" pid is ")
#define MAX_JOBS 110
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class SmallShell;
class Command {
private:
    std::string cmd_line;
protected:
    std::string get_cmd_line(){return cmd_line;}

public:
    Command(const char *cmd_line) : cmd_line(cmd_line) {}

    virtual ~Command() = default;

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();

    virtual bool is_external() const {return false;}
    virtual std::string get_name() const;
};

class BuiltInCommand : public Command {
protected:
    SmallShell* smash;
public:
    BuiltInCommand(const char *cmd_line, SmallShell* smash = NULL) : Command(cmd_line), smash(smash) {};

    ~BuiltInCommand() override = default;
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~ExternalCommand() override = default;

    void execute() override;
    bool is_external() const override {return true;}
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(const char *cmd_line, SmallShell* smash) : BuiltInCommand(cmd_line, smash){};

    virtual ~ChangePromptCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd);

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    void execute() override;
};


class JobsList {
public:
    class JobEntry {
    private:
        int id;
        pid_t pid;
        Command* cmd;
    public:
        explicit JobEntry(int id,pid_t pid, Command *cmd) : id(id),pid(pid), cmd(cmd) {}
//        JobEntry(JobEntry const &) = delete; //disable copy ctor

        int get_id() const;
        int get_pid() const;
        // bool is_deleted();
        std::string get_command_name();
        int operator==(JobEntry const &) const;
    };

    std::vector<JobEntry*> jobs; //the jobs list itself. TODO: jobs vector or pointers vector? TODO: switch to smart_ptr

    int get_new_id();
    void delete_job_by_pid(pid_t pid);
    void delete_finished_jobs();
public:
    JobsList();

    ~JobsList() = default; //TODO: memory management?

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList() const;

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
public:
    JobsCommand(const char *cmd_line, SmallShell* smash) : BuiltInCommand(cmd_line, smash){};

    virtual ~JobsCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
public:
    ChmodCommand(const char *cmd_line);

    virtual ~ChmodCommand() {}

    void execute() override;
};


class SmallShell {
private:
    pid_t smash_pid;
    std::string prompt;
    std::string curr_path, path_history;
    JobsList jobsList;
    SmallShell(); // ctor
    void delete_finished_jobs();
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

    void executeCommand(const char *cmd_line);

    void setCurrentPrompt(const std::string &new_prompt);
    const std::string &getCurrentPrompt() const;
    void printJobs() const;
};

#endif //SMASH_COMMAND_H_
