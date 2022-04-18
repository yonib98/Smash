 #ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <string>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
class Command {
  protected:
 std::string cmd_line;
friend class JobsList;
 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
  char** argv;
  bool background;
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand();
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangePromptCommand: public BuiltInCommand {
  std::string prompt;
 public:
  ChangePromptCommand(const char* cmd_line);
  virtual ~ChangePromptCommand() {}
  void execute() override;
};
class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
 std::string next_pwd;
 public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
  public:
  class JobEntry {
    std::string cmd_line;
    int job_id;
    int pid;
    bool isStopped;
    bool isFinished;
    time_t start_time;
    public:
    JobEntry(std::string cmd_line, time_t start_time, int job_id,int pid, bool isStopped=false): cmd_line(cmd_line), job_id(job_id),pid(pid), isStopped(isStopped), isFinished(false), start_time(start_time) {};
    friend class JobsList;
    friend class ForegroundCommand;
    friend class BackgroundCommand;
    friend class KillCommand;
    friend class QuitCommand;
   // TODO: Add your data members
  };
  private:
  std::list<JobEntry*> allJobs;
  std::list<JobEntry*> stoppedJobs;
  int max_job_id;
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList();
  void addJob(std::string cmd_line, int pid, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  void removeJobFromStoppedJobs(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
  int signum;
  int pid;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
  std::string cmd;
  int pid;
  int job_id;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 std::string cmd;
 int pid; 
 int job_id;
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TailCommand : public BuiltInCommand {
 public:
  TailCommand(const char* cmd_line);
  virtual ~TailCommand() {}
  void execute() override;
};

class TouchCommand : public BuiltInCommand {
 public:
  TouchCommand(const char* cmd_line);
  virtual ~TouchCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  std::string prompt; 
  char* plastpwd;
  JobsList* jobs;
  int running_pid;
  std::string running_process;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  void setPrompt(std::string prompt);
  std::string getPrompt();
  void addJob(std::string cmd_line, int pid, bool is_stopped=false);
  int getRunningPid();
  void setRunningPid(int pid);
  void setRunningProcess(std::string cmd);
  std::string getRunningProcess();
};

#endif //SMASH_COMMAND_H_
