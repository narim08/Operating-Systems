#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct prc{
	int pid;
	int arrivalTime;
	int burstTime;
	
	double waitTime; // turnaround - burst
	double turnTime; // completion - arrival
	double resTime; // start - arrival
	
	double startTime; //first run
	double remainingTime;
	double completionTime;
	
	int start;
	int complete;
} process;

int pn = 0; //process number
process proc[1000];
char ganttChart[10000][5];
double currTime = 0.0, totalIdleTime = 0.0;

void FCFS();
void SJF();
void RR(int quantum);
void SRTF();


int compareProc(const void*a, const void*b) {
	const process* p1 = (const process*)a;
	const process* p2 = (const process*)b;
	
	if(p1->arrivalTime != p2->arrivalTime) {
		return p1->arrivalTime - p2->arrivalTime;
	}
	return p1->pid - p2->pid;
}

void FCFS() {
	int slot = 0; //slot of gantt chart
	int completeProc = 0; //completed process
	int currProc = 0; //Currently selected process
	
	//Sort by arrival order
	qsort(proc, pn, sizeof(process), compareProc);
	
	while (completeProc < pn) {
		if (proc[currProc].arrivalTime <= currTime && currProc < pn) { //already arrived
			if(!proc[currProc].start) {
				proc[currProc].startTime = currTime;
				proc[currProc].start = 1; //start flag
			}
			sprintf(ganttChart[slot], "|P%d|", proc[currProc].pid);
			currTime += 1;
			proc[currProc].remainingTime -= 1;
			slot += 1;
			
			//End of process execution
			if (proc[currProc].remainingTime <= 0) {
				proc[currProc].complete = 1;
				proc[currProc].completionTime = currTime;
				completeProc += 1;
				currProc += 1; //next process
				currTime += 0.1; //add context switch time
			}
		}
		else { //Nothing has arrived yet
			sprintf(ganttChart[slot], "|x|");
			currTime += 1;
			slot += 1;
			totalIdleTime +=1;
		}
	}
}


void SJF() {
	int slot = 0; //slot of gantt chart
	int completeProc = 0; //completed process
	int currProc = -1; //Currently selected process
	
	while (completeProc < pn) {
		int shortBurst = 999;
		currProc = -1;
        	
        	 for (int i=0; i<pn; i++) {
        	 	 //Select the shortest time in the current time
        	 	 if (!proc[i].complete) { //not done
        	 	 	if(proc[i].arrivalTime <= currTime) {//already arrived
        	 	 		if (proc[i].burstTime < shortBurst) { //Update to the shortest one
        	 	 			shortBurst = proc[i].burstTime;
        	 	 			currProc = i;
        	 	 		}
        	 	 	}
        	 	 }
        	 }
        	 //Nothing has arrived yet
        	 if (currProc == -1) {
        	 	sprintf(ganttChart[slot], "|x|");
			currTime += 1;
			slot += 1;
			totalIdleTime +=1;
			continue;
        	 }
        	 //When running for the first time
        	 if (!proc[currProc].start) {
        	  	proc[currProc].startTime = currTime;
        	  	proc[currProc].start = 1; //start flag
        	 }
        	 //Execute process
        	 while (proc[currProc].remainingTime > 0) {
        	  	sprintf(ganttChart[slot], "|P%d|", proc[currProc].pid);
			currTime += 1;
			proc[currProc].remainingTime -= 1;
			slot += 1;
        	 }
        	 //End process
        	 proc[currProc].complete = 1;
		 proc[currProc].completionTime = currTime;
		 completeProc += 1;
		 currTime += 0.1; //add context switch time
	}
}


void RR(int quantum) {
    int slot = 0;
    int completeProc = 0;
    int queue[1000]; // Ready queue
    int front = 0, rear = 0;
    int currProc = -1;

    // Initialize the queue with processes that have arrived by currTime
    for (int i = 0; i < pn; i++) {
        if (proc[i].arrivalTime <= currTime) {
            queue[rear++] = i;
            proc[i].start = 1; // Mark as started
            if (proc[i].startTime < 0) {
                proc[i].startTime = currTime;
            }
        }
    }

    while (completeProc < pn) {
        if (front == rear) { // No process in the queue, CPU idle
            sprintf(ganttChart[slot], "|x|");
            currTime += 1;
            slot++;
            totalIdleTime += 1;

            // Check for newly arrived processes during idle time
            for (int i = 0; i < pn; i++) {
                if (!proc[i].start && proc[i].arrivalTime <= currTime) {
                    queue[rear++] = i;
                    proc[i].start = 1;
                    if (proc[i].startTime < 0) {
                        proc[i].startTime = currTime;
                    }
                }
            }
            continue;
        }

        // Fetch the current process from the front of the queue
        currProc = queue[front++];
        
        // Calculate execution time for the current process (either full quantum or remaining time)
        int execTime = (proc[currProc].remainingTime < quantum) ? proc[currProc].remainingTime : quantum;
        
        // Execute the current process
        for (int t = 0; t < execTime; t++) {
            sprintf(ganttChart[slot++], "|P%d|", proc[currProc].pid);
            currTime += 1;
            proc[currProc].remainingTime -= 1;
            
            // Check for any new arrivals during execution and add to the queue
            for (int i = 0; i < pn; i++) {
                if (!proc[i].start && proc[i].arrivalTime <= currTime) {
                    queue[rear++] = i;
                    proc[i].start = 1;
                    if (proc[i].startTime < 0) {
                        proc[i].startTime = currTime;
                    }
                }
            }
        }

        // If the process is complete
        if (proc[currProc].remainingTime <= 0) {
            proc[currProc].complete = 1;
            proc[currProc].completionTime = currTime;
            completeProc += 1;
        } else {
            // Process is not complete, so re-add it to the queue
            queue[rear++] = currProc;
        }

        // Add context switch time if there are other processes in the queue
        if (front != rear) {
            currTime += 0.1;
        }
    }
}



void SRTF() {
	int slot = 0; //slot of gantt chart
	int completeProc = 0; //completed process
	int currProc = -1; //Currently selected process
	int prevProc = -1;
	
	while (completeProc < pn) {
		int currProc = -1; //shortestJob
        	int shortRemaining = 999;
        
        	//Select the shortest time in the current time
        	for (int i = 0; i < pn; i++) {
            		if (!proc[i].complete) { //not done
            			if(proc[i].arrivalTime <= currTime) {
              				if (proc[i].remainingTime < shortRemaining) {
                    				shortRemaining = proc[i].remainingTime;
                    				currProc = i;
               	 		}
               	 	}
            		}
		}
		//Execute process
		if (currProc != -1) {
			if (prevProc != -1 && prevProc != currProc) {
                		currTime += 0.1;
            		}
		
            		if (!proc[currProc].start) { //Not started yet
            			proc[currProc].startTime = currTime;
            			proc[currProc].start = 1; //start flag
            		}
            		sprintf(ganttChart[slot], "|P%d|", proc[currProc].pid);
            		currTime += 1;
			proc[currProc].remainingTime -= 1;
			slot += 1;
			
			//End of process execution
			if (proc[currProc].remainingTime <= 0) {
				proc[currProc].complete = 1;
				proc[currProc].completionTime = currTime;
				completeProc += 1;
			}
			prevProc = currProc;
		}
		else { //Nothing has arrived yet
			sprintf(ganttChart[slot], "|x|");
			currTime += 1;
			slot += 1;
			totalIdleTime +=1;
		}
	}
}


int main(int argc, char*argv[]) {	
	FILE *fp = fopen(argv[1], "r"); //input.1 open
	if (!fp) { perror("file open error!"); exit(1);}
	
	memset(ganttChart, 0, sizeof(ganttChart));
	int readCnt;
	while((readCnt = fscanf(fp, "%d %d %d", &proc[pn].pid, &proc[pn].arrivalTime, &proc[pn].burstTime)) == 3) {
		proc[pn].startTime = -1;
		proc[pn].start = 0;
		proc[pn].complete = 0;
		proc[pn].remainingTime = proc[pn].burstTime;
		pn++;
	}
	fclose(fp);
	
	// ============== Select cpu scheduler ============== //
	if (strcmp(argv[2], "FCFS") == 0) { FCFS(); }
	else if (strcmp(argv[2], "SJF") == 0) { SJF(); }
	else if (strcmp(argv[2], "RR") == 0) { 
		if (argc != 4) { perror("no time quantum!"); exit(1); }
		RR(atoi(argv[3])); //time quantum
	}
	else if (strcmp(argv[2], "SRTF") == 0) { SRTF(); }
	else { perror("invalid inst!"); exit(1); }
	
	
	// ============== Print Gantt chart ============== //
	printf("Gantt Chart:\n");
	for(int i=0; ganttChart[i][0]!='\0'; i++) {
		printf("%s", ganttChart[i]);
	}
	printf("\n");
	
	
	// ============== Print average value ============== //
	double avgWaitTime = 0, avgTurnTime = 0, avgResTime = 0, cpuUtil = 0;
	double totalWaitTime = 0, totalTurnTime = 0, totalResTime =0, totalBurstTime = 0;
	
	for(int i=0; i<pn; i++) {
		//Turnaround Time
		proc[i].turnTime = proc[i].completionTime - proc[i].arrivalTime;
		totalTurnTime += proc[i].turnTime;
		
		//Waiting Time
		proc[i].waitTime = proc[i].turnTime - proc[i].burstTime;
		totalWaitTime += proc[i].waitTime;
		
		//Response Time
		proc[i].resTime = proc[i].startTime - proc[i].arrivalTime;
		totalResTime += proc[i].resTime;
		
		//Burst Time
		totalBurstTime += proc[i].burstTime;
	}
	//Average Waiting Time
	avgWaitTime = totalWaitTime / pn;
	printf("Average Waiting Time = %.2f\n", avgWaitTime);
	
	//Average Turnaround Time
	avgTurnTime = totalTurnTime / pn;
	printf("Average Turnaround Time = %.2f\n", avgTurnTime);
	
	//Average Response Time
	avgResTime = totalResTime / pn;
	printf("Average Response Time = %.2f\n", avgResTime);
	
	//CPU Utilization
	cpuUtil = 100 * (totalBurstTime / currTime);
	printf("CPU Utilization = %.2f%%\n", cpuUtil);
	
	return 0;
}
