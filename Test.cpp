#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <limits>

using namespace std;

class Timer {
public:
	int time;
	void settime(int);
	int gettime();
	void inctime(int);
};
void Timer::settime(int x) {
	time = x;
}
int Timer::gettime() {
	return time;
}
void Timer::inctime(int x) {
	time = time + x;
}


class Process {
	int deadline;
	int CT;
	string procDes;
	int index;
	int maxIndex;
	bool finished;
public:
	Process();
	int getdeadline();
	int getCT();
	void setdeadline(int);
	void setCT(int);
	void addprocDes(string);
	string getprocDes();
	int getMaxIndex();
	void setMaxIndex();
	bool getfinished();
	void setfinished(bool);
	void decCT(int);
};

Process::Process(){
	deadline = 0;
	procDes = "";
	CT = 0;
	index = 0;
	maxIndex = 0;
	finished = false;
}

bool Process::getfinished(){
	return finished;
}
void Process::decCT(int x){
	CT -= x;
}
void Process::setfinished(bool x){
	finished = x;
}

void Process::setdeadline(int x) {
	deadline = x;
}
void Process::setCT(int x) {
	CT = x;
}
int Process::getCT() {
	return CT;
}
int Process::getdeadline() {
	return deadline;
}
void Process::addprocDes(string text){
	text += "\n";
	procDes += text;
}
string Process::getprocDes(){
	return procDes;
}
int Process::getMaxIndex(){
	return maxIndex;
}
void Process::setMaxIndex(){
	maxIndex = procDes.length();
}

bool testSafety(int **, int[], int, int **, int, Process[]);
bool safe(bool[], int);
bool comp(int, int **, int[], int);
void requestMachine(int **, int[], int, int **, int, Process[], int[], int, string);
void childProcess(int **, int[], int, int **, int, Process[], int[], int);
int parentProcess(Process[], int, int);
bool testProcessFinish(Process[], int);
void instructionMachine(string, Process[], int);

Timer timeClock;

int main(int argc, char *argv[])
{

	//initializing argument for filename, the number of children processes
	//and the value by which they increment/decrement the contents of the file
	string filename = argv[1];

	int resources;
	int process;


	char* file = new char[sizeof(filename)];
	strcpy(file, filename.c_str());
	string text;
	fstream inputStream;
	inputStream.open(file);
	inputStream >> resources;
	inputStream >> process;

	timeClock.settime(0);
	Process mainProcess[process];
	int available[resources];

	for (int i = 0; i<resources; i++)
		inputStream >> available[i];

	int **Max;
	int **Alloc;
	Max = new int*[process];
	Alloc = new int*[process];
	for (int i = 0; i<process; i++) {
		Max[i] = new int[resources];
		Alloc[i] = new int[resources];
	}

	string maxdigit;
	int num;
	char temp[1];
	for (int i = 0; i<process; i++) {
		for (int j = 0; j<resources; j++) {
			inputStream >> maxdigit;
			for (int k = 0; k<maxdigit.length(); k++) {
				if (maxdigit[k] == '=') {
					temp[0] = maxdigit[maxdigit.length() - 1];
					num = atoi(temp);
					Max[i][j] = num;
				}
			}
		}
	}
	for (int i = 0; i<process; i++) {
		for (int j = 0; j<resources; j++) {
			Alloc[i][j] = 0;
		}
	}

	testSafety(Max, available, process, Alloc, resources, mainProcess);
	//After every request, we add to Alloc and thereby get a different answer

	//We begin storing the Description of each Process
	int CT;
	int deadline;
	for(int i=0; i<process; i++){
		inputStream >> text; //process_i
		inputStream >> 	deadline;
		inputStream >> CT;
		mainProcess[i].setdeadline(deadline);
		mainProcess[i].setCT(CT);
		inputStream >> text; //getting rid of 'process_1' string
		while(text != "end"){
			mainProcess[i].addprocDes(text);
			inputStream >> text;
		}
		//cout << "Process: " << i+1 << endl;
		//cout << mainProcess[i].getprocDes() << endl;
		mainProcess[i].setMaxIndex();
		//cout << mainProcess[i].getMaxIndex() << endl;
	}


	int requestArray[resources];


	int childTurn = -1;

	do{
		cout << childTurn << endl;
		childTurn = parentProcess(mainProcess, childTurn, process);
		cout << childTurn << endl;
		childProcess(Max, available, process, Alloc, resources, mainProcess, requestArray, childTurn);
	}while(parentProcess(mainProcess, childTurn, process) != -1);

	//for(int processIndex=0; processIndex<process;processIndex++)
		//childProcess(Max, available, process, Alloc, resources, mainProcess, requestArray, processIndex);

	cout << endl;
	cout << "TIme is: " << timeClock.gettime() << endl;

	inputStream.close();

	return 0;
}

int parentProcess(Process mainProcess[], int index, int process){
	int lowest = 100;
	int lowIndex;

	if(!testProcessFinish( mainProcess, process))
		return -1;
	for(int i=0;i<process;i++){
		if( mainProcess[i].getCT() < lowest && i != index && !mainProcess[i].getfinished()){
			lowest = mainProcess[i].getCT();
			lowIndex = i;
		}
	}
	return lowIndex;


}

bool testProcessFinish(Process mainProcess[], int process){
	for(int i=0; i<process;i++)
		if(!mainProcess[i].getfinished())
			return true;
	return false;

}

void childProcess(int **Max, int available[], int process, int **Alloc, int resources, Process mainProcess[], int requestArray[], int processIndex){
	int max = mainProcess[processIndex].getMaxIndex();
	string text = mainProcess[processIndex].getprocDes();
	string inst;
	int k = 0;
	cout << "\tProcess " << processIndex+1 << " runnng"<< endl;
	for(int i = 0; i<max;i++){
		inst = "";
		k = i;
		while(text[k] != ')')
			k++;
		k++; //reads in the ')'
		inst = text.substr(i,k-i);
		cout << inst << endl;
		if(inst[0] == 'r' && inst[1] == 'e' && inst[2] == 'q')
			requestMachine(Max, available, process, Alloc, resources, mainProcess, requestArray, processIndex, inst);
		else
			instructionMachine(inst, mainProcess, processIndex);
		i = k;
	}
	mainProcess[processIndex].setfinished(true);
	if(timeClock.gettime() > mainProcess[processIndex].getdeadline())
		cout<< "Passed Deadline" << endl;
	else
		cout << "Met Deadline" << endl;
}

bool testSafety(int **Max, int available[], int process, int **Alloc, int resources, Process mainProcess[])
{
	bool finish[process];
	for (int i = 0; i < process; i++)
		finish[i] = false;
	int work[resources];
	for (int i = 0; i < resources; i++)
		work[i] = available[i];

	int **Need;
	Need = new int*[process];
	for (int i = 0; i<process; i++)
		Need[i] = new int[resources];
	for (int i = 0; i<process; i++) {
		for (int j = 0; j<resources; j++) {
			Need[i][j] = Max[i][j] - Alloc[i][j];
			//cout << Need[i][j] << " ";
		}
		//cout << endl;
	}
	/*
	cout << "Available After" << endl;
	for(int i=0; i<resources; i++)
	cout<<available[i] << " ";
	cout << endl;
	cout << "Allocation After" << endl;
	for(int i=0; i<process; i++){
	for(int j=0; j<resources; j++){
	cout<< Alloc[i][j] << " ";
	}
	cout <<endl;
	}*/

	int i = 0;
	int count = 0;
	bool out = false;

	while (!safe(finish, process) && !out) {
		count++;
		//if (finish[i] == false)
			//cout << "Process " << i << "s Turn" << endl;
		//cout << comp(resources, Need, work, i);
		if (finish[i] == false && comp(resources, Need, work, i)) {
			for (int j = 0; j < resources; j++) {
				//cout << work[j] << " ";
				work[j] = work[j] + Alloc[i][j];

			}
			//cout << endl;
			finish[i] = true;
		}
		//else if (finish[i] == false)
				//cout << "Process " << i << " waited" << endl;

		if (i == process - 1 && count >9) {
			out = true;
			cout << "Unsafe" << endl;
		}
		else if (i == process - 1) {
			i = 0;
		}
		else
			i++;
	}

	for(int i=0; i<process; i++){
		for(int j=0; j<resources; j++){
			Alloc[i][j] = 0;
		}
	}
	//cout << "THIs is the " << count << endl;
	if (safe(finish, process))
		cout << "Safe" << endl;
	else
		cout << "Unsafe" << endl;

	return true;
}

void instructionMachine(string instruction, Process mainProcess[], int processIndex) {
	char test[1];
	int num;
	if(instruction[0] == 'r'){
		timeClock.inctime(1);
		mainProcess[processIndex].decCT(1);
		cout << "New CT for Process " << processIndex << ": " << mainProcess[processIndex].getCT() << endl;
	}
	else{
		for (int i = 0; i<instruction.length(); i++) {
			if (instruction[i] == '(') {
				test[0] = instruction[i + 1];
				num = atoi(test);
				num = num;
				timeClock.inctime(num);
				mainProcess[processIndex].decCT(num);
				cout << "New CT for Process " << processIndex << ": " << mainProcess[processIndex].getCT() << endl;
				i++;
			}
		}
	}
}

void requestMachine(int **Max, int available[], int process, int **Alloc, int resources, Process mainProcess[], int requestArray[], int processIndex, string request) {
	int index = 0;
	char test[1];
	for (int i = 0; i<request.length(); i++) {
		if (request[i] == '(') {
			while (request[i] != ')') {
				if (request[i] == '(' || request[i] == ',') {
					test[0] = request[i + 1];
					requestArray[index] = atoi(test);
					index++;
				}
				i++;
			}
		}
	}
	//for (int i = 0; i< resources; i++)
		//cout << requestArray[i] << " ";
	//cout << endl;

	/*cout << "Available Before" << endl;
	for(int i=0; i<resources; i++)
	cout<<available[i] << " ";
	cout << endl;
	cout << "Allocation Before" << endl;
	for(int i=0; i<process; i++){
	for(int j=0; j<resources; j++){
	cout<< Alloc[i][j] << " ";
	}
	cout <<endl;
	}*/

	//Above should be done by adjusting the String request
	int tempavailable[resources];
	for (int i = 0; i<resources; i++) {
		tempavailable[i] = available[i];
		tempavailable[i] = tempavailable[i] - requestArray[i];
		Alloc[processIndex][i] = Alloc[processIndex][i] + requestArray[i];
	}
	/*
	cout << "Available After" << endl;
	for(int i=0; i<resources; i++)
	cout<<available[i] << " ";
	cout << endl;
	cout << "Allocation After" << endl;
	for(int i=0; i<process; i++){
	for(int j=0; j<resources; j++){
	cout<< Alloc[i][j] << " ";
	}
	cout <<endl;
	}*/

	if(testSafety(Max, tempavailable, process, Alloc, resources, mainProcess)){
		mainProcess[processIndex].decCT(1);
		cout << "New CT for Process " << processIndex << ": " << mainProcess[processIndex].getCT() << endl;
	}

}




bool safe(bool finish[], int process) {
	for (int i = 0; i<process; i++)
		if (finish[i] == false)
			return false;
	return true;
}

bool comp(int resources, int **Need, int work[], int i) {
	for (int j = 0; j<resources; j++) {
		//cout <<Need[i][j] << " and  " << work[j] << endl;
		if (Need[i][j] > work[j]) {
			return false;
		}
	}
	return true;
}
