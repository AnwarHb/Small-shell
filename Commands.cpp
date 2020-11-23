#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <dirent.h>
#include <fcntl.h>
#include <algorithm>
#include <linux/limits.h>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

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
string _getCwd(){
    char myCwd[PATH_MAX];
    if (!getcwd(myCwd, sizeof(myCwd))) {
        perror("smash error: getcwd failed");
        return "";
    } else {
        return string(myCwd);
    }
}
void _ErrorPrint(string err){
    cout<< "smash error:"<< err <<endl;
}
Command::Command(const char *cmd_line, int args, char **argsArr):command_line(cmd_line),num_of_args(args) {
    //array intilization//
    for(int i=0;i<num_of_args;++i)
        arguments[i]=argsArr[i];

}
Command::~Command(){
    for(int i=0; i<num_of_args; i++){
        free(arguments[i]);
    }
}
void GetCurrDirCommand::execute() {
    //get current directory & check if it is empty//
    string current = _getCwd();
    if(current.empty())
        return;

    cout << current << endl;
}
void changePrompt::execute() {
    if(num_of_args < 2){
        SmallShell::getInstance().setprompt("smash");
    } else
        SmallShell::getInstance().setprompt(arguments[1]);
}
void ChangeDirCommand::execute() {
    if (num_of_args > 2) {
        _ErrorPrint("cd: too many arguments");
        return;
    }
    string currentDirectory = _getCwd();
    if(currentDirectory.empty())
        return;
    char* myPath= arguments[1];
    string prev = SmallShell::getInstance().getprev();
    if((strcmp(myPath, "-") == 0)){
        if(prev.empty()){
            _ErrorPrint("cd: OLDPWD not set");
            return;
        } else if(chdir(prev.c_str())) {
            perror("smash error: chdir failed");
            return;
        }
        else{
            SmallShell::getInstance().setPrev(currentDirectory);
        }

    } else if(chdir(myPath)){
        perror("smash error: chdir failed");
        return;
    } else{
        SmallShell::getInstance().setPrev(currentDirectory);
    }

}
void ShowPidCommand::execute() {
    pid_t id = SmallShell::getInstance().getShellId();
    if (!id){ //
        id = getpid();
        SmallShell::getInstance().setShellId(id);
    }
    cout  << id <<  endl;
};

void listCommand::execute() {
    struct dirent **entries;
    int numOfEntries = scandir(".", &entries, NULL, alphasort);

    if (numOfEntries < 0) /// should not reach here
        perror("scandir");
    else {
        for (int i = 0; i < numOfEntries; i++) {
            printf("%s\n", entries[i]->d_name);
            free(entries[i]);
        }
    }
    free(entries);

}

SmallShell::SmallShell() : curr("smash>") {
    shellId = getpid();
    listJobs = new JobsList();
}

SmallShell::~SmallShell() {
    delete(listJobs);
// TODO: add your implementation
}
JobsList::JobEntry::JobEntry(int jobId, string Name, JobsList::jStatus jobstatus, pid_t pid, time_t arrival):jobId(jobId),commandLine(Name),status(jobstatus),pid(pid),tao(arrival) {

}
void JobsCommand::execute(){
    SmallShell::getInstance().getJobslist()->removeFinishedJobs();
    SmallShell::getInstance().getJobslist()->printJobsList();
}
JobsList::JobEntry* JobsList::getLastJob(){
    JobEntry* lstJob=nullptr;
    int maxjid= 0;
    vector<JobEntry>::iterator start = jobsVec.begin();
    vector<JobEntry>::iterator end = jobsVec.end();
    while (start!=end){
        // last job with the highest id - find highest id and update pointer //
        if(start->getId() > maxjid){
            maxjid = start->getId();
            lstJob = &(*start);
        }
        ++start;
    }
    return lstJob;
}
void JobsList::removeFinishedJobs(){

    if(jobsVec.size() == 0)
        return;

    vector<JobEntry>::iterator start =jobsVec.begin();
    vector<JobEntry>::iterator end =jobsVec.end();
    pid_t id;
    int waitId,s = 0;

    while(start!=end){
        id = start->getPid();
        waitId = waitpid(id, &s, WNOHANG);
        if(waitId == -1){
            perror("smash error: waitpid failed");
            continue;
        } else if(id==waitId) {
            start = jobsVec.erase(start);
        } else{
            ++start;
        }
    }

    if(jobsVec.size() != 0)
        maxId = getLastJob()->getId();
    else
        maxId = 0;
}

void JobsList::killAllJobs(){
    removeFinishedJobs();
    vector<JobEntry>:: iterator start =jobsVec.begin();
    vector<JobEntry>:: iterator end =jobsVec.end();
    //iterate through jobsList & kill each job//
    while(start!=end){
        if(kill(start->getPid(),SIGKILL)==-1){
            perror("smash error: kill failed");
        }
        start++;

    }

}
void JobsList::printJobsList(){
    //first sort the jobs vector //
    sort(jobsVec.begin(), jobsVec.end());
    vector<JobEntry>::iterator start =jobsVec.begin();
    vector<JobEntry>::iterator end =jobsVec.end();
    while(start!=end){
        cout<<"[" << (start-> getId()) <<"] " << start->getName() << " : "
            << start->getPid() << " " << difftime(time(nullptr),(start)->getArrivalTime()) << " secs";

        //check if job have been stopped & print //
        if(start->getStatus() == jStatus::Stopped){
            cout<< " (stopped)";
        }
        cout << endl;
        ++start;
    }
}

void JobsList::addJob(string Name, int pid, JobsList::jStatus status) {
    this->removeFinishedJobs(); // don't need it?
    JobEntry newEnt(++maxId,Name,status,pid,time(nullptr));
    jobsVec.push_back(newEnt);

}
void ExternalCommand::execute(){
    pid_t prId = fork();
    if(prId == 0){
        char* line = {"/bin/bash", "-c", this->command_line, nullptr};
        int exResult = execvp(line[0], line);
        if(exResult == -1)
            perror("smash error: execvp failed");
    } else{
        if(wait() == -1)
            perror("smash error: wait failed");
    }

}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

    char* arguments[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, arguments);
    string cmdSS = string(arguments[0]);

    if (cmdSS == "chprompt") {
        return new changePrompt(cmd_line, num_of_args, arguments);
    } else if (cmdSS == "pwd") {
        return new GetCurrDirCommand(cmd_line, num_of_args, arguments);
    } else if (cmdSS == "cd") {
        return new ChangeDirCommand(cmd_line, num_of_args, arguments);
    } else if (cmdSS == "ls") {
        return new listCommand(cmd_line, num_of_args, arguments);
    } else if (cmdSS == "showpid") {
        return new ShowPidCommand(cmd_line, num_of_args, arguments);
    } /*else if(cmdSS == "jobs"){
        return new JobsCommand(cmd_line, num_of_args, arguments);
    }*/

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}


