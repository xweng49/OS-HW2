#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
using namespace std;

class processes
{
  int deadline;
  int comptime;
  string sprocess;
  bool finished;
public:
  void addstring(string);
  void setdeadline(int dl) {deadline = dl;}
  void setcomptime(int ct) {comptime = ct;}
  void setfinished(bool f) {finished = f;}
  void subtime(int t) {comptime -= t;};

  int getdeadline() {return deadline;}
  int getcomptime() {return comptime;}
  string getinstruct() {return sprocess;}
  bool getfinished() {return finished;}
};

class Timer
{
  int timer;
public:
  int gettime() {return timer;}
  void settime(int t) {timer=t;}
  void addtime(int t) {timer= timer+t;}

};
void processes::addstring(string line)
{
  line += "\n";
  sprocess += line;
}

bool safetest(processes[], int*, int**, int**, int, int);
bool allfinish(bool[], int);
int processturn(processes[], int, int, Timer*);
bool procallfinish (processes[], int);
bool testneed(int**, int*, int, int);
void ProcessInst(processes[], int, int, int*, int**, int**, int, Timer*);
void processreq(processes[], int, int, int*, int**, int**, int, string, Timer*);
void processother(processes[], string, int, Timer*);

int main(int argc, char *argv[]) {
  int resource;
  int process;
  int a, b;
  int *avail = NULL;
  int **max=NULL;
  int **alloc=NULL;
  int availres;
  char chars[255];
  Timer timer[1];
  timer[0].settime(0);
  fstream myfile(argv[1]);
  myfile >> resource;
  myfile >> process;
  processes myprocess[process];
  // set available matrix dimension = 1 x (resource)
  avail = new int[resource];
  //set max matrix dimension = process x resource
  max = new int*[process];
  for (int i = 0; i<process; i++)
  {
    max[i] = new int[resource];
  }
  alloc = new int*[process];
  for (int i =0; i<process; i++)
  {
    alloc[i] = new int[resource];
  }

  string line;



  //initialize the available matrix from file
  for (a = 0; a < resource; a++)
  {
    myfile >> availres;
    avail[a] = availres;
  }
  //initialize the max matrix from file
  for (a = 0; a < process; a++)
  {
    for (b = 0; b < resource; b++)
    {
      myfile >> line;
      line = line.substr(line.find("=") + 1);
      strncpy(chars, line.c_str(), sizeof (chars));
      max[a][b] = atoi(chars);
    }
  }

  //Safety Algorithm to check if need < available
  safetest(myprocess, avail, max, alloc, resource, process);

  //Save individual process commands into a string
  int deadline;
  int ct;
  for (int i=0; i<process; i++)
  {
  /*size_t pos = (contents.find("end")+3);
  pcontents = contents.substr(0, pos);
  contents = contents.substr(pos);
  cout << pcontents;*/

  myfile >> line;
  myfile >> deadline;
  myprocess[i].setdeadline(deadline);
  myfile >> ct;
  myprocess[i].setcomptime(ct);
  //cout << myprocess[i].getdeadline() <<"   " <<myprocess[i].getcomptime();
  myfile >> line;
    while(line != "end")
    {
      myprocess[i].addstring(line);
      myfile >> line;
    }
    //cout << myprocess[i].getinstruct();
  }

  //Setting turn order for process sequence
  int turn;
  do
  {
    turn=processturn(myprocess, turn, process, timer);
    if (turn != -1)
    {
      ProcessInst(myprocess, resource, process, avail, max, alloc, turn, timer);
    }
  }while (turn != -1);

  cout <<"\nTotal time for completion is :" << timer[0].gettime()<<'\n';
}


//process the turn order using least laxity first << laxity = (deadline - current time) - computation time
int processturn (processes proc[], int turn, int process, Timer timer[])
{
  int lowctime = 1000;
  int pturn = -1;
  if (!procallfinish (proc, process))
  {
    return -1;
  }
  for (int i=0; i<process; i++)
  {
    if(proc[i].getdeadline()-timer[0].gettime() -proc[i].getcomptime() < lowctime && !proc[i].getfinished() )
    {
    lowctime = proc[i].getdeadline() -timer[0].gettime() - proc[i].getcomptime();
    pturn = i;
    cout << "\nProcess " << pturn + 1 << " lax time is currently at " << lowctime;
    }
  }

  return pturn;
}


//tests whether process is currently in safe state
bool safetest(processes proc[], int* avail, int** max, int** alloc, int resource, int process)
{
  int work[resource];
  bool finish[process] = {0};


  for (int i=0; i<resource; i++)
  {
    work[i] = avail[i];
  }

  int** need;
  need = new int *[process];
  for(int i=0; i<process; i++)
  {
    need[i]=new int [resource];
  }

  for (int i=0; i<process; i++)
  {
    for (int j=0; j<resource; j++)
    {
      need[i][j] = max[i][j]-alloc[i][j];

    }
  }

  int pno=0;
  bool done=false;

  while(!allfinish(finish, process) && !done)
  {

    if (finish[pno]==false && testneed(need, work, resource, pno))
    {
      for(int i=0; i<resource; i++)
      {
        work[i] = work[i] + alloc[pno][i];
      }
    finish[pno]=true;
    }
    if(pno == process)
    {
      done = true;
      cout << "Not in safe state";
    }
    else
      pno++;
  }



  if (allfinish(finish, process))
  {
    cout << "\nSafe State";
  }
  else
  {
    cout << "Unsafe State";
  }
}

//tests if each process has gone through the loop for work=work+alloc
bool allfinish(bool finish[], int process)
{
  for (int i=0; i<process; i++)
  {
    if (finish[i] == false)
    {
      return false;
    }
  }
  return true;
}

//tests if all process has been completed
bool procallfinish (processes proc[], int process)
{
  for(int i=0; i<process; i++)
  {
    if(!proc[i].getfinished())
    {
      return true;
    }
  }
  return false;
}

//segment of the safety test, makes sure need < work
bool testneed(int** need, int* work, int resource, int pno)
{
  for (int i=0; i<resource; i++)
  {
    if(need[pno][i] > work[i])
    {
      return false;
    }
  }
  return true;
}


//process the instructions from a string and decides whether the instruction is a request instruction or other instruction
void ProcessInst(processes proc[], int resource, int process, int* avail, int** max, int** alloc, int turn, Timer timer[])
{
  string contents= proc[turn].getinstruct();
  string inst;
  int request[resource];
  char cnum;
  int rnum;
  int ra;
  size_t pos;
  cout << "\nOn Process: " << turn + 1 <<"\n";
  for(int i=0; i<=(proc[turn].getinstruct()).length(); i++)
  {
    pos = (contents.find(")")+1);
    inst=contents.substr(0, pos);
    i+=pos;
    contents = contents.substr(pos);
    cout << inst;

    if (inst.find("req") != -1)
    {
      processreq(proc, resource, process, avail, max, alloc, turn, inst, timer);

    }
    else
    {
      processother(proc, inst, turn, timer);
    }
    break;
  }
  for (int i=0; i<process; i++)
  {
    if(proc[i].getcomptime() <= 0)
    {
      proc[i].setfinished(true);
    }
  }
}


//process instruction for request and compares deadline with total time if the computation reaches 0
void processreq(processes proc[], int resource, int process, int* avail, int** max, int** alloc, int turn, string inst, Timer timer[])
{
  char rnum;
  int request[resource];
  int a;

  for (int i; i<inst.length(); i++)
  {
    while(inst[i]!=')')
    {
      if(inst[i] == '(' || inst[i] == ',')
      {
        rnum = inst[i+1];
        request[a] = rnum - '0';
        request[a];
        a++;
      }
    i++;
    }
  }
  proc[turn].subtime(1);
  timer[0].addtime(1);
  safetest(proc, avail, max, alloc, resource, process);
  cout << "\nTime is: " << timer[0].gettime();
  cout << "\nNew Computation Time for process "<< turn + 1 <<" is:" << proc[turn].getcomptime();
  if(proc[turn].getcomptime() <= 0)
  {
    if(proc[turn].getdeadline() >= timer[0].gettime())
    {
      cout << "\nTime is " << timer[0].gettime();
      cout << "\nProcess " << turn + 1 << " completed within the deadline of " << proc[turn].getdeadline();
    }
    else  if(proc[turn].getdeadline() < timer[0].gettime())
    {
      cout << "\nProcess " << turn + 1 << " did not meet the deadline of " << proc[turn].getdeadline();
    }
  }

}

//process instructions for !request and compares deadline with total time if the computation reaches 0
void processother(processes proc[], string inst, int turn, Timer timer[])
{
  int t;
  char ct;
  size_t pos;
      if (inst.find("rel") != -1)
      {
        t=1;
        proc[turn].subtime(t);
        timer[0].addtime(t);
      }
      else
      {
        pos = inst.find("(")+1;
        ct=inst[pos];
        t=ct-'0';
        proc[turn].subtime(t);
        timer[0].addtime(t);

      }
  cout << "\nTime is: " << timer[0].gettime();
  cout << "\nNew Computation Time for process "<< turn  + 1<<" is:" << proc[turn].getcomptime();

  if(proc[turn].getcomptime() <= 0)
  {
    if(proc[turn].getdeadline() >= timer[0].gettime())
    {
      cout << "\nProcess " << turn + 1 << " completed within the deadline of " << proc[turn].getdeadline();
    }
    else  if(proc[turn].getdeadline() < timer[0].gettime())
    {
      cout << "\nProcess " << turn + 1 << " did not meet the deadline of " << proc[turn].getdeadline();
    }
  }
}
