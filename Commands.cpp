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

SmallShell::SmallShell(): prompt("smash"), plastpwd(nullptr), running_pid(-1){
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
  void SmallShell::addJob(std::string cmd_line, int pid, bool is_stopped){
    jobs->addJob(cmd_line,pid,is_stopped);
  }

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0) {
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
 // .....
  else {
    return new ExternalCommand(cmd_line);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  delete cmd;
  // Please note that you must fork smash process for some commands (e.g., external commands....)
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
//Constructors
Command::Command(const char* cmd_line): cmd_line(cmd_line){
};
BuiltInCommand::BuiltInCommand(const char* cmd_line): Command(cmd_line) {};
ExternalCommand::ExternalCommand(const char* cmd_line): Command(cmd_line) {
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

ExternalCommand::~ExternalCommand(){
  for(int i=0;i<3;i++){
    delete argv[i];
  }
  delete [] argv;
}

//BuiltIns C'tor
ChangeDirCommand::ChangeDirCommand (const char* cmd_line, char** plastPwd): BuiltInCommand(cmd_line){
  char** args = new char*[COMMAND_MAX_ARGS];
  int len =_parseCommandLine(cmd_line,args);
  if(len>2){
    throw exception(); //MORE THAN ONE ARG
  }
  if(len==0){
    throw exception(); //NO ARGS
  }
  else{
    char* path = args[1];
    if(strcmp(path,"-")==0){
      if(*plastPwd==nullptr){
        throw exception(); //OLDPWD_NOTSET
    }
        next_pwd=*plastPwd;
    }
    else{
      next_pwd = path;
    }
    if(*plastPwd==nullptr){
      *plastPwd= new char[1024];
    }
    getcwd(*plastPwd,1024);
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
      jobs->removeJobById(job_id);
      
    }else{
      job_id = atoi(args[1]);
     typename JobsList::JobEntry* job =  jobs->getJobById(job_id);
      pid = job->pid;
      cmd = job->cmd_line;
      jobs->removeJobById(job_id);

    }
    delete [] args;
}
BackgroundCommand::BackgroundCommand(const char* cmd_line,JobsList* jobs): BuiltInCommand(cmd_line){
    char** args = new char*[COMMAND_MAX_ARGS];
    int len =_parseCommandLine(cmd_line,args);
    if(len==1){
      typename JobsList::JobEntry* job;
      try{
        int jobID;
         job = jobs->getLastStoppedJob(&jobID);
      }
      catch(std::exception& e){
        //catch (does not exist)
        //catch (empty)
      }   
      pid= job->pid;
      cmd = job->cmd_line;
      jobs->removeJobFromStoppedJobs(job->job_id);
      job->isStopped=false;   
    }
    else{
      job_id = atoi(args[1]);
      typename JobsList::JobEntry* job;
     try{
        job=jobs->getJobById(job_id);
     }
     catch(std::exception& e){

     }       //job id does not exist;
      if(!job->isStopped){
        throw std::exception();//job is already in the background
      }   
      pid = job->pid;
      cmd = job->cmd_line;
      jobs->removeJobFromStoppedJobs(job_id);
      job->isStopped=false;
  }
}
//Destructors
Command::~Command(){
}
//Executes:

//BuiltIns Executes
void GetCurrDirCommand::execute(){
  char buff[1024];
   getcwd(buff,1024);
   string pwd = string(buff);
  std::cout << pwd << std::endl;
}

void ChangeDirCommand::execute(){
  int result = chdir(next_pwd.c_str());
  if(result==-1){
    throw exception(); //CHDIR failed
  }
}

void ShowPidCommand::execute(){
  int pid = getpid();
  std::cout << "Smash pid is " << pid << std::endl;
}

void ChangePromptCommand::execute(){
    SmallShell& smash = SmallShell::getInstance();
    smash.setPrompt(prompt);

}
void JobsCommand::execute(){
  jobs->printJobsList();
}
//External Executes
void ExternalCommand::execute(){
  
  int pid = fork();
  if(pid==0){
    setpgrp();
    execv("/bin/bash",argv);
  }else{
    if(!background){
    //foregound
    SmallShell& smash= SmallShell::getInstance();
    smash.setRunningPid(pid);
    smash.setRunningProcess(this->cmd_line);
    int status;
    waitpid(pid,&status,WUNTRACED);
    smash.setRunningPid(-1);
    smash.setRunningProcess("");
    }
    else{
      SmallShell& smash= SmallShell::getInstance();
      smash.addJob(this->cmd_line,pid);
    }
  }
}

void ForegroundCommand::execute(){
  std::cout << cmd<<" : "<< pid<<std::endl;
  SmallShell& smash= SmallShell::getInstance();
  smash.setRunningPid(pid);
  smash.setRunningProcess(cmd);
  int status;
  waitpid(pid,&status,WUNTRACED);
  smash.setRunningPid(-1);
  smash.setRunningProcess("");
  waitpid(pid, &status, WUNTRACED);
}

void BackgroundCommand::execute(){
  std::cout<< cmd<< " : "<< pid<< std::endl;
  kill(pid,SIGCONT);
}
//Jobs class
JobsList::JobsList(): max_job_id(0) {

}

JobsList::~JobsList(){}
void JobsList::addJob(std::string cmd, int pid,bool isStopped){
  time_t start_time = time(nullptr);
  JobEntry* new_job = new JobEntry(cmd,start_time,max_job_id+1,pid,isStopped);
  allJobs.push_back(new_job);
  if(isStopped){
    stoppedJobs.push_back(new_job);
  }
  max_job_id++;
}
void JobsList::printJobsList(){
  time_t curr_time= time(nullptr);
  for(JobEntry* tmp : allJobs){
    std::cout<<"["<<tmp->job_id<<"]"<< tmp->cmd_line<<" :"<<tmp->pid<<" "<< difftime(curr_time,tmp->start_time)<< " ";
    if(tmp->isStopped==true){
      std::cout<<"(stopped)"<<std::endl;
    }
    else
      std::cout<<std::endl;
  }
}
void JobsList::killAllJobs(){

}
void JobsList:: removeFinishedJobs(){
  
}
typename JobsList::JobEntry* JobsList::getJobById(int jobId){
  for(JobEntry* tmp : allJobs){
    if(tmp->job_id==jobId){
      return tmp;
    }
  }
  throw exception(); //Doesnt exist
}
void JobsList::removeJobById(int jobId){
  JobEntry* to_remove = getJobById(jobId);
  if(to_remove->isStopped){
    stoppedJobs.remove(getJobById(jobId));
  }
  allJobs.remove(getJobById(jobId));
}

void JobsList::removeJobFromStoppedJobs(int jobId){
  JobEntry* to_remove = getJobById(jobId);
  stoppedJobs.remove(getJobById(jobId));
}

typename JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){
  JobEntry* lastjob= allJobs.back();
  *lastJobId=lastjob->job_id;
  return lastjob;

}
typename JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
  JobEntry* lastStoppedJob= stoppedJobs.back();
  *jobId= lastStoppedJob->job_id;
  return lastStoppedJob;
  
}
