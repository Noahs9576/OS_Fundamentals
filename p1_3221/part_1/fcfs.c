#include <stdio.h>
#include <stdlib.h>

#include "sch-helpers.h"  

process processes[MAX_PROCESSES+1];   
int numberOfProcesses=0;              

 int main(int argc, char *argv[]) {
    int status;
    int currentTime=0;
    int contextSwitches=0;
    int lastProcess=-1;

    double totalWaitTime=0;
    double totalTurnaroundTime=0;

    while (status=readProcess(&processes[numberOfProcesses]))  {
         if(status==1)  numberOfProcesses ++;
    } 

    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    for (int i = 0; i < numberOfProcesses; i++) {
        process *p = &processes[i];
        int turnaroundTime = 0;

        if (currentTime < p->arrivalTime) {
            currentTime = p->arrivalTime;
        }

        p->startTime = currentTime;
        p->waitingTime = currentTime - p->arrivalTime;

        for (int j = 0; j < p->numberOfBursts; j++) {
            currentTime += p->bursts[j].length;
        }

        p->endTime = currentTime;
        turnaroundTime = p->endTime - p->arrivalTime;

        totalWaitTime += p->waitingTime;
        totalTurnaroundTime += turnaroundTime;

        lastProcess = p->pid;

    }

    double avgWaitTime = totalWaitTime / numberOfProcesses;
    double avgTurnaroundTime = totalTurnaroundTime / numberOfProcesses;
    double cpuUtilization = (double)currentTime / (currentTime + totalWaitTime) * 100;

    fprintf(stdout, "Avg. Waiting Time: %.2f\n", avgWaitTime);
    fprintf(stdout, "Avg. Turnaround Time: %.2f\n", avgTurnaroundTime);
    fprintf(stdout, "Total Time: %d\n", currentTime);
    fprintf(stdout, "CPU Utilization: %.2f%% \n", cpuUtilization);
    fprintf(stdout, "Context Switches: %d\n", contextSwitches);
    fprintf(stdout, "Last PID: %d\n", lastProcess);

 }

