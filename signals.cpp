#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
  std::cout <<"smash: got ctrl-Z"<<std::endl;
  SmallShell& smash = SmallShell::getInstance();
  int pid = smash.getRunningPid();
  std::string running_process= smash.getRunningProcess();
  if(pid!=-1){
    smash.addJob(running_process,pid,true);
    int success_sys=kill(pid,SIGSTOP);
    if(success_sys==-1){
      perror("smash errorkill failed");
      return;
    }
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
  std::cout << "smash:" << "got an alarm" << std::endl;
  SmallShell& smash = SmallShell::getInstance();
  AlarmEntry* timeout_alarm = smash.popAlarm();
  std::cout << "smash timeout: " << timeout_alarm->getPid() << "has timeout, sending kill" << std::endl;
  delete timeout_alarm;
  int success_sys = kill(timeout_alarm->getPid(),SIGKILL);
    if(success_sys==-1){
      perror("smash error: kill failed");
      return;
    }
}

