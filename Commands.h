 #ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <queue>
#include <string>
#include <stdio.h>
#include <exception>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
using std::exception;

class AlarmEntry{
  time_t timestamp;
  int duration;
  int pid;
  std::string command_to_execute;
  public:
  friend class ExternalCommand;
  friend class TimeoutCommand;
  friend class SmallShell;
  bool operator< (AlarmEntry& e){
    return this->duration < e.duration;
  }
  int getPid() {return pid;};
  std::string getCommandToExecute() {return command_to_execute;};
};

class Comparator{
  public:
  bool operator() (AlarmEntry* palrm1,AlarmEntry* palrm2){
    return (*palrm1 < *palrm2);
  }
};

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
  AlarmEntry* alarm;
 public:
  ExternalCommand(const char* cmd_line, AlarmEntry* alarm=nullptr);
  virtual ~ExternalCommand();
  void execute() override;
  void setAlarm(AlarmEntry* alarm) {this->alarm=alarm;};
};

class PipeCommand : public Command {
  bool redirect_errors;
  Command* first_command;
  Command* second_command;
 public:
  PipeCommand(const char* cmd_line, bool redirect_errors);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 bool append;
 Command* command;
 std::string filename;
 int flags;
 mode_t mode;
 public:
  explicit RedirectionCommand(const char* cmd_line, bool append);
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
 char** plastPwd;
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
  bool killAll;
  JobsList* jobs;
  public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
  public:
  class JobEntry {
    public:
    std::string cmd_line;
    int job_id;
    int pid;
    bool isStopped;
    bool isFinished;
    bool FG;
    time_t start_time;
    JobEntry(std::string cmd_line, time_t start_time, int job_id,int pid, bool isStopped=false,bool FG=false): cmd_line(cmd_line), job_id(job_id),pid(pid), isStopped(isStopped), isFinished(false), start_time(start_time),FG(FG) {};
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
  void addJob(std::string cmd_line, int pid, bool isStopped = false, bool FG=false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  void removeJobFromStoppedJobs(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  friend class SmallShell;
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
  bool isStopped;
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
  std::string filename;
  int lines;
 public:
  TailCommand(const char* cmd_line);
  virtual ~TailCommand() {}
  void execute() override;
};

class TouchCommand : public BuiltInCommand {
  std::string filename;
  time_t timestamp;
 public:
  TouchCommand(const char* cmd_line);
  virtual ~TouchCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand{
  int duration;
  Command* command;
  std::string command_to_execute;
  public:
    TimeoutCommand(const char* cmd_line);
    virtual ~TimeoutCommand() {}
    void execute() override;
};


class SmallShell {
 private:
  std::string prompt; 
  char* plastpwd;
  JobsList* jobs;
  int running_pid;
  int pid;
  std::string running_process;
  int running_job_id;
  std::vector<AlarmEntry*> alarms;
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
  void addJob(std::string cmd_line, int pid, bool is_stopped=false,bool FG=false);
  int getRunningPid();
  void setRunningPid(int pid);
  void setRunningProcess(std::string cmd);
  std::string getRunningProcess();
  int getPid();
  void setRunningJobId(int job_id){ this->running_job_id=job_id;}
  int getRunningJobId(){return running_job_id;}
  JobsList::JobEntry* getJobById(int job_id) {return jobs->getJobById(job_id);}
  int getMaxJobId(){return jobs->max_job_id;}
  void addAlarmEntry(AlarmEntry* alarm);
  AlarmEntry* popAlarm();

};

class InvalidArgs: public exception {
  std::string command_name;
  public:
  InvalidArgs(std::string command_name):  command_name(command_name) {};
  std::string getCommand() {return command_name;}
};

class JobIdNotExist: public exception {
  std::string command_name;
  int job_id;
  public:
  JobIdNotExist(std::string command_name, int job_id): command_name(command_name), job_id(job_id){};
  std::string getCommand() {return command_name;}
  int getJobid() {return job_id;}
};
class FGJobListEmpty: public exception{};

class StoppedJobsEmpty: public exception{
  public:
  StoppedJobsEmpty() {};
};

class AlreadyBackgroundCommand: public exception{
  int job_id;
  public:
  AlreadyBackgroundCommand(int job_id): job_id(job_id){};
  int getJobid(){return job_id;}
};
class CDTooManyArguments: public exception {};
class CDOLDPWDNotSet: public exception {};


#endif //SMASH_COMMAND_H_
