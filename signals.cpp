#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
  SmallShell& smash = SmallShell::getInstance();
  int pid = smash.getRunningPid();
  std::string running_process= smash.getRunningProcess();
  if(pid!=-1){
	  std::cout <<"smash: got ctrl-Z"<<std::endl;
    smash.addJob(running_process,pid,true);
    kill(pid,SIGSTOP);
    std::cout <<"smash: process "<<pid<<" was stopped"<< std::endl;
  }
}

void ctrlCHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    int pid = smash.getRunningPid();
    if(pid!=-1){
      std::cout <<"smash: got ctrl-C"  << std::endl;
      kill(pid,SIGKILL);
      std::cout << "smash: process "<< pid<< " was killed"<< std::endl;
    }
}

void alarmHandler(int sig_num) {
  std::cout << "smash:" << "got an alarm" << std::endl;
  SmallShell& smash = SmallShell::getInstance();
  int pid = smash.getRunningPid();
}

