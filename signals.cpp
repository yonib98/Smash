#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

void ctrlZHandler(int sig_num) {
  std::cout <<"smash: got ctrl-Z"<<std::endl;
  SmallShell& smash = SmallShell::getInstance();
  int pid = smash.getRunningPid();
  std::string running_process= smash.getRunningProcess();
  if(pid!=-1){
    int running_job=smash.getRunningJobId();
    JobsList::JobEntry* job = smash.getJobById(running_job);
    if(running_job==-1){
      job->job_id=smash.getMaxJobId()+1;
    }
    int success_sys=kill(pid,SIGSTOP);
    if(success_sys==-1){
        perror("smash errorkill failed");
        return;
      }
    job->start_time=time(NULL);
    job->isStopped=true;
    job->FG=false;
    smash.addJobToStoppedJobs(job);
    std::cout <<"smash: process "<<pid<<" was stopped"<< std::endl;
  }
}

void ctrlCHandler(int sig_num) {
    std::cout <<"smash: got ctrl-C"  << std::endl;
    SmallShell& smash = SmallShell::getInstance();
    int pid = smash.getRunningPid();
    if(pid!=-1){
      int success_sys = kill(pid,SIGKILL);
      if(success_sys==-1){
        perror("smash error: kill failed");
        return;
      }
      std::cout << "smash: process "<< pid<< " was killed"<< std::endl;
    }
}

void alarmHandler(int sig_num) {
  std::cout << "smash:" << " got an alarm" << std::endl;
  SmallShell& smash = SmallShell::getInstance();
  AlarmEntry* timeout_alarm = smash.popAlarm();
  
  smash.jobs->removeFinishedJobs();
  int success_sys = kill(timeout_alarm->getPid(),SIGKILL);
    if(success_sys==-1){
     // perror("smash error: kill failed");
      return;
    }
      std::cout << "smash: " << timeout_alarm->getCommandToExecute() << " timed out!" << std::endl;
      delete timeout_alarm;
}

