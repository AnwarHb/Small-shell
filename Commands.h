#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

class Command {
protected:
    const char* command_line;
    int num_of_args;
    char* arguments[COMMAND_MAX_ARGS];
// TODO: Add your data members
 public:
  Command(const char* cmd_line,int args,char* argsArr[COMMAND_MAX_ARGS]);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line,int args,char** argsArr):Command(cmd_line,args,argsArr){}
  ~BuiltInCommand()  override=default;
};
class JobsList;
class ExternalCommand : public Command {
    JobsList* jobs;
 public:
  explicit ExternalCommand(const char* cmd_line,int argsNum,char** arguments,JobsList* jobs):Command(cmd_line,argsNum,arguments),jobs(jobs){}
  ~ExternalCommand()  override=default;
  void execute() override;
};
//no change //
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

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  explicit ChangeDirCommand(const char* cmd_line,int argsNum, char** args):BuiltInCommand(cmd_line,argsNum,args){}
  ~ChangeDirCommand() override =default;
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  explicit GetCurrDirCommand(const char* cmd_line,int argsNum,char** args):BuiltInCommand(cmd_line,argsNum,args){}
  ~GetCurrDirCommand() override=default;
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line,int argsNum,char** args):BuiltInCommand(cmd_line,argsNum,args){}
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

class CommandsHistory {
 protected:
  class CommandHistoryEntry {
	  // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  CommandsHistory();
  ~CommandsHistory() {}
  void addRecord(const char* cmd_line);
  void printHistory();
};

class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};

class JobsList {

 public:
    enum class jStatus{Running,Stopped}; //status for job
  class JobEntry {
      int jobId;
      string commandLine;
      jStatus status;
      pid_t pid;
      time_t toa;//time of arrival
  public:
      JobEntry(int jobId,string Name,jStatus jobstatus,pid_t pid,time_t arrival);
      ~JobEntry()=default;
      int getId(){
          return this->jobId;
      }
      string getName(){
          return this->commandLine;
      }
      jStatus getStatus(){return this->status;}
      void setStatus(jStatus otherStatus){
          this->status=otherStatus;
      }
      pid_t getPid(){
          return this->pid;
      }
      time_t getArrivalTime(){
          this->toa;
      }
      bool operator==(const JobEntry& other) const{
          return (this->jobId==other.jobId);
      }
      bool operator<(const JobEntry& other) const{
          return (this->jobId<other.jobId);
      }

  };
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList();
  void addJob(string Name,int pid,jStatus status);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob();
  JobEntry *getLastStoppedJob();
  // TODO: Add extra methods or modify exisitng ones as needed
private:
    vector<JobEntry> jobsVec;
    int maxId;
    JobEntry* x;
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line,int argsNum,char** args, JobsList* jobs):BuiltInCommand(cmd_line,argsNum,args){}
  ~JobsCommand() override=default;
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

// TODO: add more classes if needed 
// maybe ls, timeout
class listCommand: public BuiltInCommand{
public:
    explicit listCommand(const char* cmd_line,int argsNum,char** args):BuiltInCommand(cmd_line,argsNum,args){}
    ~listCommand() override=default;
    void execute() override;
};
class changePrompt: public  BuiltInCommand{
public:
    explicit changePrompt(const char* cmd_line,int argsNum,char** args):BuiltInCommand(cmd_line,argsNum,args){}
    ~changePrompt() override=default;
    void execute() override;
};

class SmallShell {
 private:
    pid_t shellId;
    string curr;//current prompt
    string prev;
    JobsList* listJobs;

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
  // TODO: add extra methods as needed
  pid_t getShellId(){
      return shellId;
  }
  void setShellId(pid_t newid){
      shellId=newid;
  }
  string getCurr(){
      return curr;
  }
  JobsList* getJobslist(){
      return listJobs;
  }
  void setprompt(const char* newPrompt){
      curr = string(newPrompt) + ">";
  }
  string getprev(){
      return prev;
  }
  void setPrev(string Other){
      prev=Other;
  }
};

#endif //SMASH_COMMAND_H_
