#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <memory>

#define ERROR_PROMPT std::string("smash error: ")
#define DEFAULT_PROMPT std::string("smash")
#define PROMPT_SUFFIX std::string("> ")
#define PID_IS std::string(" pid is ")
#define MAX_JOBS 110
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define MIN_SIGNUM 0
#define MAX_SIGNUM 32

class SmallShell;
class Command {
private:
    std::string cmd_line;
    bool bg_command;
protected:
    std::string get_cmd_line(){return cmd_line;}

public:
    Command(std::string cmd) : cmd_line(cmd + " "),bg_command(false) {}

    virtual ~Command() = default;

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();

    virtual bool is_external() const {return false;}
    bool isBgCommand() const{return this->bg_command;}
    virtual std::string get_name() const;
};

class BuiltInCommand : public Command {
protected:
    void smash_print(const std::string input);
    void smash_error(const std::string input);
public:
    BuiltInCommand(std::string cmd_line) : Command(cmd_line){};

    ~BuiltInCommand() override = default;
};

class ExternalCommand : public Command {
public:
    ExternalCommand(std::string cmd_line) : Command(cmd_line) {};

    virtual ~ExternalCommand() override = default;

    void execute() override;
    bool is_external() const override {return true;}
};


class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(std::string cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(std::string cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(std::string cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~ChangePromptCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(std::string cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~ChangeDirCommand() = default;

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(std::string cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(std::string cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
public:
    QuitCommand(std::string cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~QuitCommand() {}

    void execute() override;
};


class JobsList {
public:
    class JobEntry {
    private:
        int id;
        pid_t pid;
        std::string cmd;
    public:
        explicit JobEntry(int id, pid_t pid, std::string cmd) : id(id),pid(pid), cmd(cmd) {}
//        JobEntry(JobEntry const &) = delete; //disable copy ctor

        int get_id() const;
        int get_pid() const;
        // bool is_deleted();
        std::string get_command_name();
        int operator==(JobEntry const &) const;
    };

    std::vector<std::shared_ptr<JobEntry>> jobs;

    int get_new_id();
    void delete_job_by_pid(pid_t pid);
    void delete_job_by_id(int jobId);
    void delete_finished_jobs();
public:
    JobsList();

    ~JobsList() = default; //TODO: memory management?

    void addJob(std::string cmd, pid_t pid);

    void printJobsList() const;

    void killAllJobs();

    void removeFinishedJobs();

    std::shared_ptr<JobEntry> getJobById(int jobId);
    std::shared_ptr<JobEntry> getJobByPid(const int& jobPid) const;
    void removeJobById(int jobId);

    std::shared_ptr<JobsList> getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
public:
    JobsCommand(std::string cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~JobsCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    KillCommand(std::string cmd_line)  : BuiltInCommand(cmd_line){};

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
public:
    ForegroundCommand(std::string cmd_line)  : BuiltInCommand(cmd_line){};

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
public:
    ChmodCommand(std::string cmd_line);

    virtual ~ChmodCommand() {}

    void execute() override;
};


class SmallShell {
private:
    pid_t smash_pid;
    std::string prompt;
    std::string prev_path;
    JobsList jobsList;
    SmallShell(); // ctor
    void delete_finished_jobs();
public:
    std::shared_ptr<Command> CreateCommand(std::string cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }


    ~SmallShell();

    void executeCommand(std::string cmd_line);

    void smash_print(const std::string input);
    void smash_error(const std::string input);
    void setCurrentPrompt(const std::string &new_prompt);
    const std::string &getCurrentPrompt() const;
    int get_num_jobs() const;
    void printJobs() const;
    void killall();
    std::shared_ptr<JobsList::JobEntry> getJobById(int Id);
    pid_t getPidById(int Id);
    std::string &getPrevPath();
    void addJob(std::string cmd, pid_t pid);
    void deleteJob(pid_t pid);
};

#endif //SMASH_COMMAND_H_
