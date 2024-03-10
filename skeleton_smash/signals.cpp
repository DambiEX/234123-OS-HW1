#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
  SmallShell::getInstance().ctrlCHandler(sig_num);
}

void alarmHandler(int sig_num) {
  cout << "smash: got an alarm" << endl;
}

