#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <stdio.h>
#include <algorithm>

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



//SmallShell
SmallShell::SmallShell(): prompt("smash"), plastpwd(nullptr), running_pid(-1){
  pid = getpid();
  if(pid==-1){
    perror("smash error: getpid failed");
  }
  jobs= new JobsList();
}

SmallShell::~SmallShell() {
  delete [] plastpwd;
  delete jobs;
}

void SmallShell::setPrompt(string prompt){
  this->prompt=prompt;
}

string SmallShell::getPrompt(){
  return this->prompt;
}

void SmallShell::addJob(std::string cmd_line, int pid, bool is_stopped,bool FG){
  jobs->addJob(cmd_line,pid,is_stopped,FG);
}

void SmallShell::addJobToStoppedJobs(JobsList::JobEntry* job){
  jobs->addToStoppedJobs(job);
}

void SmallShell::setRunningPid(int pid){
  running_pid=pid;
}

int SmallShell::getRunningPid(){
  return running_pid;
}
void SmallShell::setRunningProcess(std::string cmd){
  this->running_process=cmd;
}

std::string SmallShell::getRunningProcess(){
  return this->running_process;
}
int SmallShell::getPid(){
  return pid;
}
//adding cooment
void SmallShell::addAlarmEntry(AlarmEntry* new_alarm){

  time_t current_time = time(NULL);
  for(AlarmEntry* tmp : alarms){
    tmp->duration -= (int)difftime(current_time,tmp->timestamp);
    tmp->timestamp=current_time;
  }
  this->alarms.push_back(new_alarm);
    AlarmEntry* closest_alarm = *(std::min_element(alarms.begin(),alarms.end(),Comparator()));
  alarm(closest_alarm->duration);
}
AlarmEntry* SmallShell::popAlarm(){
  std::vector<AlarmEntry*>::iterator closest_alarm = std::min_element(alarms.begin(),alarms.end(),Comparator());
  AlarmEntry* timedout_alarm = *(closest_alarm);
  alarms.erase(closest_alarm);
  if(alarms.empty()){
    return timedout_alarm;
  }
  time_t current_time = time(NULL);
  for(AlarmEntry* tmp : alarms){
    tmp->duration -= (int)difftime(current_time,tmp->timestamp);
    tmp->timestamp=current_time;
  }
  AlarmEntry* new_alarm = *(std::min_element(alarms.begin(),alarms.end(),Comparator()));
  alarm(new_alarm->duration);
  return  timedout_alarm;
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line, std::string sent_from_timeout) {
	// For example:

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if(cmd_s.find("|&",0)!= string::npos){
    return new PipeCommand(cmd_line, true);
  }
  else if(cmd_s.find("|",0)!=string::npos){
    return new PipeCommand(cmd_line,false);
  }
  else if(cmd_s.find(">>",0)!= string::npos){
    return new RedirectionCommand(cmd_line, true);
  }
  else if (cmd_s.find(">",0)!=string::npos){
    return new RedirectionCommand(cmd_line,false);
  }
  else if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&")==0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt")==0 || firstWord.compare("chprompt&")==0){
    return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("cd")==0 || firstWord.compare("cd&")==0){
    return new ChangeDirCommand(cmd_line, &this->plastpwd);
    
  } 
  else if (firstWord.compare("jobs")==0 || firstWord.compare("jobs&")==0){
    return new JobsCommand(cmd_line,jobs);
  }
  else if(firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0){
    return new ForegroundCommand(cmd_line,jobs);
  }
  else if(firstWord.compare("bg") == 0 || firstWord.compare("bg&") == 0){
    return new BackgroundCommand(cmd_line,jobs);
  }
  else if (firstWord.compare("kill")==0 || firstWord.compare("kill&")==0){
    return new KillCommand(cmd_line,jobs);
  }
  else if (firstWord.compare("quit")==0 || firstWord.compare("quit&")==0){
   return new QuitCommand(cmd_line,jobs);
  }
  else if (firstWord.compare("touch")==0 || firstWord.compare("touch&")==0){
    return new TouchCommand(cmd_line);
  }
  else if (firstWord.compare("tail")==0 || firstWord.compare("tail&")==0){
    return new TailCommand(cmd_line);
  }
  else if (firstWord.compare("timeout")==0 || firstWord.compare("timeout&") == 0){
    return new TimeoutCommand(cmd_line);
  }
 // .....
  else {
    return new ExternalCommand(cmd_line, sent_from_timeout);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  try{
    jobs->removeFinishedJobs();
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    delete cmd;
  }
  catch(JobIdNotExist& e){
    std::cerr << "smash error: " << e.getCommand() << ": job-id " << e.getJobid() << " does not exist"<< std::endl;
  }
  catch(FGJobListEmpty& e){
    std::cerr << "smash error: fg: jobs list is empty" << std::endl;
  }
  catch(AlreadyBackgroundCommand& e){
    std::cerr << "smash error: bg: job-id "<<e.getJobid() << " is already running in the background"<< std::endl; 
  }
  catch(StoppedJobsEmpty& e){
    std::cerr << "smash error: bg: there is no stopped jobs to resume"<< std::endl;
  }
  catch (CDTooManyArguments& e){
    std::cerr << "smash error: cd: too many arguments" << std::endl;
  }
  catch (CDOLDPWDNotSet& e){
    std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
  }
  catch (InvalidArgs& e){
    std::cerr << "smash error: " << e.getCommand() << ": invalid arguments" << std::endl;
  }
  
}

//FINISHED SMASH


//------Commands----------
//BASIC COMMANDS Constructors
Command::Command(const char* cmd_line): cmd_line(cmd_line){}

BuiltInCommand::BuiltInCommand(const char* cmd_line): Command(cmd_line) {};

ExternalCommand::ExternalCommand(const char* cmd_line,std::string sent_from_timeout,AlarmEntry* alarm): Command(cmd_line),sent_from_timeout(sent_from_timeout), alarm(alarm) {
  this->cmd_line= (sent_from_timeout.length()==0)? cmd_line: sent_from_timeout+ " " +this->cmd_line;
  argv = new char*[4];
  argv[0] = new char[20];
  argv[1]= new char[3];
  argv[2] = new char[strlen(cmd_line)+1];
  strcpy(argv[0],"/bin/bash");
  strcpy(argv[1],"-c");
  strcpy(argv[2], cmd_line);
  if(_isBackgroundComamnd(cmd_line)){
    _removeBackgroundSign(argv[2]);
    background=true;
  }
  else {
    background=false;
  }
  argv[3]=nullptr;
};

//BuiltIns C'tor
ChangeDirCommand::ChangeDirCommand (const char* cmd_line, char** plastPwd): BuiltInCommand(cmd_line), plastPwd(plastPwd){
  char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
  if(len>2){
    throw CDTooManyArguments();
  }
  else{
    char* path = args[1];
    if(strcmp(path,"-")==0){
      if(*plastPwd==nullptr){
        throw CDOLDPWDNotSet();
    }
        next_pwd=*plastPwd;
    }
    else{
      next_pwd = path;
    }
 }
  
  delete [] args;
}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line) {};

ShowPidCommand::ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){};

ChangePromptCommand::ChangePromptCommand(const char* cmd_line): BuiltInCommand(cmd_line) {
    char** args = new char*[COMMAND_MAX_ARGS];
    int len =_parseCommandLine(cmd_line,args);
    if(len==1){
      prompt="smash";
    }else{
      prompt=args[1];
    }
    delete [] args;
};

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){};

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line){
    char** args = new char*[COMMAND_MAX_ARGS];
    int len =_parseCommandLine(cmd_line,args);
    if(len==1){
     typename JobsList::JobEntry* job = jobs->getLastJob(&job_id);
      pid = job->pid;
      cmd = job->cmd_line;
      isStopped= job->isStopped;
      job->FG=true;
      SmallShell& smash=SmallShell::getInstance();
      smash.setRunningJobId(job->job_id);
    }
    else if(len==2){
      job_id = atoi(args[1]);
      if(job_id==0){
        throw InvalidArgs(args[0]);
      }
      typename JobsList::JobEntry* job;
      try{
        job =  jobs->getJobById(job_id);
      }
      catch (FGJobListEmpty& e){
        throw JobIdNotExist(args[0], job_id);
      }
      if(job==nullptr){
        throw JobIdNotExist(args[0],job_id);
      }
      pid = job->pid;
      cmd = job->cmd_line;
      isStopped = job->isStopped;
      job->FG=true;
      SmallShell& smash=SmallShell::getInstance();
      smash.setRunningJobId(job->job_id);
    }
    else{
      throw InvalidArgs(args[0]);
    }
    delete [] args;
}

BackgroundCommand::BackgroundCommand(const char* cmd_line,JobsList* jobs): BuiltInCommand(cmd_line){
    char** args = new char*[COMMAND_MAX_ARGS];
    int len =_parseCommandLine(cmd_line,args);
    if(len==1){
      typename JobsList::JobEntry* job;
      int jobID;
      job = jobs->getLastStoppedJob(&jobID);
      pid= job->pid;
      cmd = job->cmd_line;
      jobs->removeJobFromStoppedJobs(job->job_id);
      job->isStopped=false;   
    }
    else if (len==2){
      job_id = atoi(args[1]);
      if(job_id==0){
        throw InvalidArgs(args[0]);
      }
      typename JobsList::JobEntry* job;
      try{
      job=jobs->getJobById(job_id);
      if(job==nullptr){
        throw JobIdNotExist(args[0],job_id);
      }
      }
      catch (FGJobListEmpty& e){
        throw JobIdNotExist(args[0],job_id);
      }
      if(!job->isStopped){
        throw AlreadyBackgroundCommand(job_id);//job is already in the background
      }   
      pid = job->pid;
      cmd = job->cmd_line;
      jobs->removeJobFromStoppedJobs(job_id);
      job->isStopped=false;
  delete args;
  }
  else{
    throw InvalidArgs(args[0]);
  }
}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line){
  char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
  if(len!=3){
    throw InvalidArgs(args[0]);
  }
  int job_id = atoi(args[2]);
  if(job_id==0){
        throw InvalidArgs(args[0]);
  }
  signum=atoi(args[1])*(-1);
  if(signum<=0 || signum>64){
    throw InvalidArgs(args[0]);
  }
  typename JobsList::JobEntry* job;
  try{
    job = jobs->getJobById(job_id);
  }
 catch (FGJobListEmpty& e){
   throw JobIdNotExist(args[0], job_id);
 }
  if(job==nullptr){
    throw JobIdNotExist(args[0], job_id);
  }
  pid = job->pid;

  delete [] args;
}

QuitCommand::QuitCommand(const char* cmd_line,JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){
  char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
  if(len==1 || strcmp(args[1],"kill")!=0){
    killAll=false;
  }else{
    killAll=true;
  }
  delete [] args;
}

//----Special I/O Commands----

RedirectionCommand::RedirectionCommand(const char* cmd_line, bool append): Command(cmd_line),append(append){
  std::string command_n;
  if(!append){
  std::string command_f = _trim(string(cmd_line));
  command_n =_trim(command_f.substr(0,command_f.find_first_of(">")));
  filename =_trim(command_f.substr(command_f.find_first_of(">")+1,command_f.length()-1));
  }
  else{
  std::string command_f = _trim(string(cmd_line));
  command_n = _trim(command_f.substr(0,command_f.find_first_of(">>")));
  filename =_trim(command_f.substr(command_f.find_first_of(">>")+2,command_f.length()-1));
  }
  SmallShell& shell= SmallShell::getInstance();
  command= shell.CreateCommand(command_n.c_str());
  if(append){
    flags= O_WRONLY | O_APPEND | O_CREAT;
  }
  else{
    flags= O_WRONLY | O_CREAT | O_TRUNC;
  }
  mode= 0655;
}

PipeCommand::PipeCommand(const char* cmd_line, bool redirect_errors): Command(cmd_line), redirect_errors(redirect_errors){
  std::string command_1;
  std::string command_2;
  if(!redirect_errors){
  std::string command_f = _trim(string(cmd_line));
  command_1 =_trim(command_f.substr(0,command_f.find_first_of("|")));
  command_2 =_trim(command_f.substr(command_f.find_first_of("|")+1,command_f.length()-1));
  }
  else{
  std::string command_f = _trim(string(cmd_line));
  command_1 = _trim(command_f.substr(0,command_f.find_first_of("|&")));
  command_2 =_trim(command_f.substr(command_f.find_first_of("|&")+2,command_f.length()-1));
  }
  SmallShell& shell= SmallShell::getInstance();
  first_command= shell.CreateCommand(command_1.c_str());
  second_command=shell.CreateCommand(command_2.c_str());
}

//--------Specail Commands C'tor---------
TouchCommand::TouchCommand(const char* cmd_line): BuiltInCommand(cmd_line){
  char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
  if(len!=3){
    throw InvalidArgs(args[0]);
  }
  filename = args[1];
  std::string given_time = args[2];
  std::istringstream iss = std::istringstream(args[2]);
  std::string tmp;
  int times[6];
  int i=0;
  while(std::getline(iss,tmp,':')){
    char* end;
    times[i]=strtol(tmp.c_str(), &end, 10);
    if(tmp.c_str()==end){
     throw InvalidArgs(args[0]);
   }
    i++;
  }
  tm temp={0};
  temp.tm_sec=times[0]; 
  temp.tm_min=times[1];
  temp.tm_hour=times[2];
  temp.tm_mday=times[3];
  temp.tm_mon=times[4]-1;
  temp.tm_year=times[5]-1900;
  timestamp = mktime(&temp);
  delete args;
}
TailCommand::TailCommand(const char* cmd_line): BuiltInCommand(cmd_line){
    char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
  if(len==2){
    filename = args[1];
    lines=10;
  }
  else if (len==3){
    char* end;
    lines = strtol(args[1], &end, 10);
    if(args[1]==end){
      throw InvalidArgs(args[0]);
    }
    lines=lines*-1;
    if(lines<0){
      throw InvalidArgs(args[0]);
    }
    filename = args[2];
  }
  else{
    throw InvalidArgs(args[0]);
  }
  delete args;
}

//---------------BONUS------------
TimeoutCommand::TimeoutCommand(const char* cmd_line): BuiltInCommand(cmd_line){
  char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
//Add exceptions
  char* end;
  duration = strtol(args[1], &end, 10);
  if(args[1]==end){
    throw InvalidArgs(args[0]);
  }
  command_to_execute=cmd_line;
  SmallShell& shell = SmallShell::getInstance();
  std::string to_send= args[0];
  to_send+= " ";
  to_send+=to_string(duration);
  command = shell.CreateCommand(cmd_line+strlen(args[0])+strlen(args[1])+2, to_send);

  delete [] args;
}
//FINISHED BUILT IN CONSTRUCTORES

//-------Destructors--------
Command::~Command(){
}

ExternalCommand::~ExternalCommand(){
  for(int i=0;i<3;i++){
    delete argv[i];
  }
  delete [] argv;
}

//-------Executes-------

//BuiltIns Executes
void GetCurrDirCommand::execute(){
  char buff[1024];
   char* success_sys=getcwd(buff,1024);
   if(success_sys==nullptr){
     perror("smash error: getcwd failed");
     return;
   }
   string pwd = string(buff);
  std::cout << pwd << std::endl;
}

void ChangeDirCommand::execute(){
  char* currdir= new char [1024];
  getcwd(currdir,1024);
  int result = chdir(next_pwd.c_str());
  if(result==-1){
    perror("smash error: chdir failed");
    return;
  }
  if(*plastPwd==nullptr){
      *plastPwd= new char[1024];
  }
    strcpy(*plastPwd, currdir);
}

void ShowPidCommand::execute(){
  SmallShell& smash= SmallShell::getInstance();
  int pid = smash.getPid();
  if(pid==-1){
    perror("smash error: getpid failed");
    return;
  }
  std::cout << "smash pid is " << pid << std::endl;
}

void ChangePromptCommand::execute(){
    SmallShell& smash = SmallShell::getInstance();
    smash.setPrompt(prompt);

}
void JobsCommand::execute(){
  jobs->removeFinishedJobs();
  jobs->printJobsList();
}

void ForegroundCommand::execute(){
  std::cout << cmd<<" : "<< pid<<std::endl;
  SmallShell& smash= SmallShell::getInstance();
  if(isStopped){
    int result_sys =kill(pid,SIGCONT);
    if(result_sys==-1){
      perror("smash error: kill failed");
      return;
    }
  }
  smash.setRunningPid(pid);
  smash.setRunningProcess(cmd);
  int status;
  int success_sys=waitpid(pid,&status,WUNTRACED);
  if(WIFEXITED(status)|| WIFSIGNALED(status)){
    smash.getJobById(job_id)->isFinished=true;
  }
  if(success_sys==-1){
    perror("smash error: waitpid failed");
    return;
  }
  smash.setRunningPid(-1);
  smash.setRunningProcess("");
}

void BackgroundCommand::execute(){
  std::cout<< cmd<< " : "<< pid<< std::endl;
  int success=kill(pid,SIGCONT);
  if(success==-1){
    perror("smash error: kill failed");
    return;
  }
}

void KillCommand::execute(){
  //Should use macro
  int success=kill(pid,signum);
  if(success==-1){
    perror("smash error: kill failed");
    return; //hasn't sent a signal...
  }
  std::cout << "signal number " << signum << " was sent to pid " << pid << std::endl;

}

void QuitCommand::execute(){
  if(killAll){
    jobs->killAllJobs();
  }
  exit(0);
}

//External Executes
void ExternalCommand::execute(){
  
  int pid = fork();
  if(pid==-1){
    perror("smash error: fork failed");
    return;
  }
  if(pid==0){
    int sys_result=setpgrp();
    if(sys_result==-1){
      perror("smash error: setpgrp failed");
      return;
    }
    int sucess_sys=execv("/bin/bash",argv);
    if(sucess_sys==-1){
      perror("smash error: execv failed");
      return;
    }
  }else{
    //father

    SmallShell& smash= SmallShell::getInstance();
    if(alarm!=nullptr){
      alarm->pid = pid;
      smash.addAlarmEntry(alarm);
    }
    if(!background){
    //foregound
    smash.setRunningPid(pid);
    smash.setRunningProcess(this->cmd_line);
    smash.setRunningJobId (-1);
    smash.addJob(this->cmd_line,pid,false,true);
    int status;
    int success_sys=waitpid(pid,&status,WUNTRACED);
    if(WIFEXITED(status) || WIFSIGNALED(status)){
      smash.getJobById(-1)->isFinished=true;
    }
    if(success_sys==-1){
      perror("smash error: waitpid failed");
      return;
    }
    smash.setRunningPid(-1);
    smash.setRunningProcess("");
    }
    else{
      smash.addJob(this->cmd_line,pid);
    }
  }
}


//-----------I/O------------
void RedirectionCommand::execute(){
  int tmp_stdout = dup(1);
  if(tmp_stdout==-1){
    perror("smash error: dup failed");
    return;
  }
  int close_suc=close(1);
  if(close_suc==-1){
    perror("smash error: close failed");
    return;
  }
  int fd= open(filename.c_str(),flags,mode);//opens to
  if(fd==-1){
    perror("smash error: open failed");
    return;
  }
  command->execute();
  delete command;
  close_suc=close(1);
  if(close_suc==-1){
    perror("smash error: close failed");
    return;
  }
  int dup_success= dup(tmp_stdout);
  if(dup_success==-1){
    perror("smash error: dup failed");
    return;
  }
  close_suc=close(tmp_stdout);
  if(close_suc==-1){
    perror("smash error: close failed");
    return;
  }
}

void PipeCommand::execute(){
  int channel = redirect_errors? 2:1;
  int fields[2];
  int result = pipe(fields);
  if(result==-1){
    perror("smash error: pipe failed");
    return;
  }
  int first_pid = fork();
  if(first_pid==-1){
    perror("smash error: fork failed");
    return;
  }
  int sys_res;
  if(first_pid==0){
    //first command
   sys_res =  close(channel);
   if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
    sys_res=dup(fields[1]);
     if(sys_res==-1){
     perror("smash error: dup failed");
     return;
   }
    sys_res=close(fields[1]);
     if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
    sys_res=close(fields[0]);
     if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
    first_command->execute();
    exit(0);
  }
  int second_pid=fork();
  if(second_pid==-1){
    perror("smash error: fork failed");
    return;
  }
  if(second_pid==0){
    sys_res=close(0);
     if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
    sys_res=dup(fields[0]);
     if(sys_res==-1){
     perror("smash error: dup failed");
     return;
   }
    sys_res=close(fields[0]);
     if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
    sys_res=close(fields[1]);
     if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
    second_command->execute();
    exit(0);
  }
 sys_res= close(fields[0]);
  if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
 sys_res= close(fields[1]);
  if(sys_res==-1){
     perror("smash error: close failed");
     return;
   }
  int status;
  sys_res=waitpid(first_pid,&status,0);
   if(sys_res==-1){
     perror("smash error: waitpid failed");
   }
  sys_res=waitpid(second_pid, &status,0);
   if(sys_res==-1){
     perror("smash error: waitpid failed");
   }

}
  //---------Special Commands----------
void TouchCommand::execute(){
  utimbuf new_time = {timestamp,timestamp};
  int success_sys=utime(filename.c_str(),&new_time);
  if(success_sys==-1){
    perror("smash error: utime failed");
    return;
  }
}

void TailCommand::execute(){
  int flags = O_RDONLY;
  int fd = open(filename.c_str(),flags);
  if(fd==-1){
    perror("smash error: open failed");
    return;
  }
  int offset = 0;
  int size = lseek(fd,offset,SEEK_END);
  if(size==0){
    return;
  }
  if(size==-1){
    perror("smash error: lseek failed");
    return;
  }
  char byte[1]={'\n'};
  while(byte[0]=='\n'){
    int tmp = lseek(fd,offset,SEEK_END);
    if(tmp==-1){
      perror("smash error: lseek failed");
      return;
    }
    int result =read(fd,byte,1);
    if(result==-1){
      perror("smash error: read failed");
      return;
    }
    offset--;
  }
  while(lines>0 && size+offset>=0){
    lseek(fd,offset,SEEK_END);
    int result = read(fd,byte,1);
    if(result==-1){
      perror("smash error: read failed");
      return;
    }
    if(byte[0]=='\n'){
      lines--;
    }
    offset--;
  }
  char* buff = new char[-offset];
  int result = read(fd,buff ,-offset);
  if(result==-1){
    perror("smash error: read failed");
    return;
  }
  
  result = write(1,buff,-offset);
  if(result==-1){
    perror("smash error: write failed");
    return;
  }
 
}

//-----------BONUS_TIMEOUT--------
void TimeoutCommand::execute(){
  AlarmEntry* new_alarm = new AlarmEntry();
  new_alarm->duration = duration;
  new_alarm->command_to_execute=command_to_execute;
  time_t current_time= time(nullptr);
  if(current_time==-1){
    perror("smash error: time failed");
  }
  new_alarm->timestamp=current_time;
  dynamic_cast<ExternalCommand*>(command)->setAlarm(new_alarm);
  command->execute();

}

//----Jobs class--------
JobsList::JobsList(): max_job_id(0) {

}

JobsList::~JobsList(){}
void JobsList::addJob(std::string cmd, int pid,bool isStopped,bool FG){
  this->removeFinishedJobs();
  time_t start_time = time(nullptr);
  if(start_time==-1){
    perror("smash error: time failed");
    return;
  }
  JobEntry* new_job;
  if(FG){
   new_job = new JobEntry(cmd,start_time,-1,pid,isStopped,FG);
  }else{
  new_job = new JobEntry(cmd,start_time,max_job_id+1,pid,isStopped,FG);
  max_job_id++;
  }
  allJobs.push_back(new_job);
  if(isStopped){
    stoppedJobs.push_back(new_job);
  }
}
void JobsList::addToStoppedJobs(JobEntry* job){
  stoppedJobs.push_back(job);
}

void JobsList::printJobsList(){
  time_t curr_time= time(nullptr);
  if(curr_time==-1){
    perror("smash error: time failed");
    return;
  }
  for(JobEntry* tmp : allJobs){
    if(tmp->job_id==-1 || tmp->FG){
      continue;
    }
    std::cout<<"["<<tmp->job_id<<"] "<< tmp->cmd_line<<" : "<<tmp->pid<<" "<< difftime(curr_time,tmp->start_time)<< " secs";
    if(tmp->isStopped==true){
      std::cout<<" (stopped)"<<std::endl;
    }
    else
      std::cout<<std::endl;
  }
}
void JobsList::killAllJobs(){
  std::cout<< "smash: sending SIGKILL signal to " << allJobs.size() <<" jobs:" << std::endl;
  for(JobEntry* tmp : allJobs){
    int pid = tmp->pid;
    int success_sys=kill(pid,9);
    if(success_sys==-1){
      perror("smash error: kill failed");
      return;
    }
    std::cout << pid << ": " << tmp->cmd_line <<  std::endl;
  }
  removeFinishedJobs();
}

void JobsList:: removeFinishedJobs(){
  std::list<JobEntry*>::iterator it= allJobs.begin(); 
  while(it!=allJobs.end()){
    int status;
    if(!(*it)->FG){
    int pid = waitpid((*it)->pid,&status,WNOHANG);
    if(pid==-1){
     perror("smash error: waitpid failed");
     continue;
   }
    if(pid!=0){
    (*it)->isFinished=true;
    it=allJobs.erase(it);
    }
    else{
    it++;
   }
  }
   else{
    if((*it)->isFinished){
      it=allJobs.erase(it);
    }
    else{
    it++;
    }
  }
  }
   it= stoppedJobs.begin(); 
   while(it!=stoppedJobs.end()){
   if((*it)->isFinished){
    it=stoppedJobs.erase(it);
   }
   else{
   it++;
   }
  }
  if(allJobs.empty()){
    max_job_id=0;
  }else{
    max_job_id = allJobs.back()->job_id;
  }
}

typename JobsList::JobEntry* JobsList::getJobById(int jobId){
  if(this->allJobs.size()==0){
    throw FGJobListEmpty();
  }
  for(JobEntry* tmp : allJobs){
    if(tmp->job_id==jobId){
      return tmp;
    }
  }
  return nullptr; //Doesnt exist
}

void JobsList::removeJobById(int jobId){
  JobEntry* to_remove = getJobById(jobId);
  if(to_remove->isStopped){
    stoppedJobs.remove(getJobById(jobId));
  }
  allJobs.remove(getJobById(jobId));
  if(allJobs.empty()){
    max_job_id=0;
  }else{
  max_job_id=allJobs.back()->job_id;
  }
}

void JobsList::removeJobFromStoppedJobs(int jobId){
  JobEntry* to_remove = getJobById(jobId);
  stoppedJobs.remove(to_remove);
}

typename JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){
  if(allJobs.size()==0){
    throw FGJobListEmpty();
  }
  JobEntry* lastjob= allJobs.back();
  *lastJobId=lastjob->job_id;
  return lastjob;

}
typename JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
  if(stoppedJobs.size()==0){
    throw StoppedJobsEmpty();
  }
  JobEntry* lastStoppedJob= stoppedJobs.back();
  *jobId= lastStoppedJob->job_id;
  return lastStoppedJob;
}
