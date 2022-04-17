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

SmallShell::SmallShell(): prompt("smash"), plastpwd(nullptr){
// TODO: add your implementation
}

SmallShell::~SmallShell() {
  delete [] plastpwd;
}
void SmallShell::setPrompt(string prompt){
  this->prompt=prompt;
}
string SmallShell::getPrompt(){
  return this->prompt;
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
  strcpy(argv[2],cmd_line);
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

//External Executes
void ExternalCommand::execute(){
  
  int pid = fork();
  if(pid==0){
    execv("/bin/bash",argv);
  }else{
    int status;
    waitpid(pid,&status,WUNTRACED);
  }
}
//Jobs class
JobsList::JobsList(): max_job_id(0) {

}

JobsList::~JobsList(){}
void JobsList::addJob(Command* cmd, int pid,bool isStopped){
  time_t start_time = time(nullptr);
  JobEntry new_job = JobEntry(cmd,start_time,max_job_id+1,pid,isStopped);
  allJobs.push_back(new_job);
  if(isStopped){
    stoppedJobs.push_back(new_job);
  }
  max_job_id++;
}
void JobsList::printJobsList(){
  time_t curr_time= time(nullptr);
  for(JobEntry a : allJobs){
    std::cout<<"["<<a.job_id<<"]"<< a.cmd<<" :"<<a.pid<<" "<< difftime(a.start_time,curr_time)<< " ";
    if(a.isStopped==true){
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
  for(JobEntry tmp : allJobs){
    if(tmp.job_id==jobId){
      return &tmp;
    }
  }
  throw exception() //Doesnt exist
}
void JobsList::removeJobById(int jobId){
  allJobs.remove(*getJobById(jobId));
  for(JobEntry a: allJobs){
    if(a.job_id=jobId){
      
    }
  }

}
typename JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){

}
typename JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
  
}