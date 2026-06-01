#include<iostream>
#include<vector>
#include<queue>
#include <algorithm>
using namespace std;
class Process {
    private:
        int pid;
        int arrival_time;
        int burst_time;
        int remaining_time;
        int completion_time;
        int turnaround_time;
        int waiting_time;
        int response_time;
        int priority;

    public:

        Process() {}
        Process(int p, int a_t, int b_t) { //SJF constructor
            pid = p;
            arrival_time = a_t;
            burst_time = b_t;
            remaining_time = b_t;
        }
        Process(int p, int a_t, int b_t,int pr) { //Priority constructor
            pid = p;
            arrival_time = a_t;
            burst_time = b_t;
            remaining_time = b_t;
            priority = pr;
        }

        int getPid()            { return pid; }
        int getArrivalTime()    { return arrival_time; }
        int getBurstTime()      { return burst_time; }
        int getRemainingTime()  { return remaining_time; }
        int getWaitingTime()    { return waiting_time; }
        int getTurnaroundTime() { return turnaround_time; }
        int getCompletionTime() { return completion_time; }
        int getpriority()       { return priority; }


        void setRemainingTime(int t)  { remaining_time = t; }
        void setCompletionTime(int t)  { completion_time = t; }

        void calculateTimes () {
            turnaround_time = completion_time - arrival_time;
            waiting_time = turnaround_time - burst_time;
        }
};




struct Compare {
    int mode ;

    Compare (int m) {
        mode = m ;
    }

    bool operator()(Process& a, Process& b) {
        switch(mode){

         case 1:             // SJF scheduling
            return a.getRemainingTime() > b.getRemainingTime();
            break;

         case 2:            // Priority scheduling

            if (a.getpriority() != b.getpriority()){
                return a.getpriority() > b.getpriority() ;
             }else{
                return a.getArrivalTime() > b.getArrivalTime();
             }
             break;
        }

        return false;
    }
};

/*int main() {
    vector<Process> allProcesses;
    vector<Process> finishedProcesses;
    vector<int> gantt;

    int current_time = 0;
    int mode;
    int submode;
    int number_of_process;
    int quantum;

    cout << "Enter the Mode you want the Scheduling in" <<endl ;
    cout << "1. SJF Scheduling " << endl;
    cout << "2. Priority Scheduling" << endl;
    cout << "3. Round Robin Scheduling" << endl;
    cout << "4. FCFS Scheduling\n"<< endl;

    cin >> mode;

    while(mode != 1 && mode != 2 && mode != 3 && mode != 4){
        cout << "Enter The correct Mode" << endl;
        cin >> mode;
    }

    if (mode == 3) {
        cout << "Enter Time Quantum:\n";
        cin >> quantum;
    }

    cout << "Enter the number of processes" << endl;
    cin >> number_of_process;

    for(int i = 0 ; i < number_of_process  ;i++ ) {
        int at , bt , pr;
        switch(mode){
            case 1:
                cout << "\n-- SJF Mode Selected --" << endl;
                cout << "Enter the (Arrival time , Burst Time) for Process "<<i+1<< endl ;
                cin >> at >> bt ;
                allProcesses.push_back(Process(i+1, at, bt));
            break;

            case 2:
                cout << "\n-- Priority Scheduling Mode Selected --" << endl;
                cout << "Enter the (Arrival time , Burst Time , Priority) for Process "<<i+1<< endl ;
                cin >> at >> bt >> pr ;
                allProcesses.push_back(Process(i+1, at, bt, pr));
            break;

            case 3:
                cout << "\n-- Round Robin Scheduling --" << endl;
                cout << "Enter the (Arrival time , Burst Time ) for Process "<<i+1<< endl ;
                cin >> at >> bt;
                allProcesses.push_back(Process(i+1, at, bt));
            break;

            case 4:
                cout << "\n-- FCFS Scheduling --" << endl;
                cout << "Enter (Arrival Time, Burst Time) for Process " << i+1 << endl;
                cin >> at >> bt;
                allProcesses.push_back(Process(i+1, at, bt));
                break;
            }
    }




    if(mode == 1 || mode == 2) {
            priority_queue<Process, vector<Process>, Compare> readyQueue{Compare(mode)};
            cout << "Enter the SubMode you want the Scheduling in" << endl;
            cout << "1. Non-preemptive" <<endl ;
            cout << "2. Preemptive" <<endl;
            cin >> submode;

            while(submode != 1 && submode != 2){
            cout << "Enter The correct SubMode" <<endl;
            cin >> submode;
            }

            while (!allProcesses.empty() || !readyQueue.empty()) {

                for (int i=0; i<allProcesses.size(); i++) {
                    if (allProcesses[i].getArrivalTime() == current_time) {
                        readyQueue.push(allProcesses[i]);
                        allProcesses.erase(allProcesses.begin()+i);
                        i--;
                    }
                }

                switch (submode) {
                case 1: // NON-PREEMPTIVE VERSION
                    if (!readyQueue.empty())
                    {
                        Process current = readyQueue.top();
                        readyQueue.pop();
                        // check arrivals for every tick during execution
                        for (int t = 0; t < current.getBurstTime(); t++)
                        {
                            gantt.push_back(current.getPid());
                            current_time++;
                            // check if any process arrives during this burst
                            for (int i = 0; i < allProcesses.size(); i++)
                            {
                                if (allProcesses[i].getArrivalTime() == current_time)
                                {
                                    readyQueue.push(allProcesses[i]);
                                    allProcesses.erase(allProcesses.begin() + i);
                                    i--;
                                }
                            }
                        }

                        current.setCompletionTime(current_time);
                        current.calculateTimes();
                        finishedProcesses.push_back(current);
                    }
                    else
                    {
                        gantt.push_back(0);
                        current_time++;
                    }
                    break;

                case 2: // PREEMPTIVE VERSION
                    if (!readyQueue.empty())
                    {
                        Process current = readyQueue.top();
                        readyQueue.pop();
                        gantt.push_back(current.getPid()); // for the part of the gantt chart
                        current.setRemainingTime(current.getRemainingTime()- 1);

                        if(current.getRemainingTime() == 0)
                        {

                            current.setCompletionTime(current_time + 1);
                            current.calculateTimes();
                            finishedProcesses.push_back(current);

                        }
                        else
                        {
                            readyQueue.push(current);
                        }
                    }
                    else
                    {
                        gantt.push_back(0);// 0 = CPU idle
                    }
                    current_time++;
                    break;
                }
            }

    } else if (mode == 3) {
            queue<Process> rrQueue;

            sort(allProcesses.begin(), allProcesses.end(),[](Process a, Process b) {
                     return a.getArrivalTime() < b.getArrivalTime();
                 });

            int i = 0;

            while (i < allProcesses.size() || !rrQueue.empty()) {
                while (i < allProcesses.size() && allProcesses[i].getArrivalTime() <= current_time) {
                    rrQueue.push(allProcesses[i]);
                    i++;
                }

                if (rrQueue.empty()) {
                    gantt.push_back(0);
                    current_time++;
                    continue;
                }

                Process current = rrQueue.front();
                rrQueue.pop();

                int runTime = min(current.getRemainingTime(), quantum);

                for (int t = 0; t < runTime; t++) {
                    gantt.push_back(current.getPid());
                    current_time++;
                    current.setRemainingTime(current.getRemainingTime() - 1);

                    while (i < allProcesses.size() && allProcesses[i].getArrivalTime() <= current_time) {
                        rrQueue.push(allProcesses[i]);
                        i++;
                    }
                }

                if (current.getRemainingTime() > 0) {
                    rrQueue.push(current);
                }
                else {
                    current.setCompletionTime(current_time);
                    current.calculateTimes();
                    finishedProcesses.push_back(current);
                }
            }
      } else if (mode == 4) {
                queue<Process> fcfsQueue;

                sort(allProcesses.begin(), allProcesses.end(),[](Process a, Process b) {
                         return a.getArrivalTime() < b.getArrivalTime();
                     });

                int i = 0;

                while (i < allProcesses.size() || !fcfsQueue.empty()) {
                    // add arrived processes
                    while (i < allProcesses.size() && allProcesses[i].getArrivalTime() <= current_time) {
                        fcfsQueue.push(allProcesses[i]);
                        i++;
                    }

                    if (fcfsQueue.empty()) {
                        gantt.push_back(0);
                        current_time++;
                        continue;
                    }

                    // take first process (FIFO)
                    Process current = fcfsQueue.front();
                    fcfsQueue.pop();

                    // run FULL burst (non-preemptive)
                    while (current.getRemainingTime() > 0) {
                        gantt.push_back(current.getPid());
                        current_time++;
                        current.setRemainingTime(current.getRemainingTime() - 1);

                        // check arrivals during execution
                        while (i < allProcesses.size() && allProcesses[i].getArrivalTime() <= current_time) {
                            fcfsQueue.push(allProcesses[i]);
                            i++;
                        }
                    }

                    current.setCompletionTime(current_time);
                    current.calculateTimes();
                    finishedProcesses.push_back(current);
                }
            }

    //============================OUTPUT======================(msh hanghyr fee haga)
    cout << "\nGantt Chart:\n| ";
    for (int i = 0; i < gantt.size(); i++) {
        if (gantt[i] == 0)
            cout << "Idle | ";
        else
            cout << "P " << gantt[i] << " | ";
        }
    cout << endl;

    // time line
    for (int i = 0; i <= gantt.size(); i++) {
            if(i<10){
                cout << i << "     ";
                }else{cout << i << "    ";}
            }
    cout << endl;



    int total_waiting = 0 , total_turnaround = 0;
    cout << "Processes' info in order " << endl;

    for (int i=0; i<finishedProcesses.size(); i++) {
        cout << "Process " << i+1 << " PID is " << finishedProcesses[i].getPid() << endl;
        cout << "Process " << i+1 << " arrival time is " << finishedProcesses[i].getArrivalTime() <<endl;
        cout << "Process " << i+1 << " burst time is " << finishedProcesses[i].getBurstTime() <<endl;
        cout << "Process " << i+1 << " waiting time is " << finishedProcesses[i].getWaitingTime() <<endl;
        cout << "Process " << i+1 << " turnaround time is " << finishedProcesses[i].getTurnaroundTime() <<endl;
        cout << "Process " << i+1 << " completion time is " << finishedProcesses[i].getCompletionTime() <<endl;

        total_waiting += finishedProcesses[i].getWaitingTime();
        total_turnaround += finishedProcesses[i].getTurnaroundTime();
    }

    cout << "Average waiting time is " << (float)total_waiting/number_of_process << endl;
    cout << "Average turnaround time is " << (float)total_turnaround/number_of_process << endl;
}*/

