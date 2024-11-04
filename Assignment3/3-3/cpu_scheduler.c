#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <queue>

//process structure that records pid, time, and flag
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
	
	int start; //start flag
	int complete; //end flag
} process;

int pn = 0; //process number
process proc[1000]; //ready queue
char ganttChart[10000][5];
double currTime = 0.0, totalIdleTime = 0.0;

void FCFS();
void SJF();
void RR(int quantum);
void SRTF();


//qsort function
int compareProc(const void*a, const void*b) {
	const process* p1 = (const process*)a;
	const process* p2 = (const process*)b;
	
	//sort by arrival
	if(p1->arrivalTime != p2->arrivalTime) {
		return p1->arrivalTime - p2->arrivalTime;
	}
	return p1->pid - p2->pid; //if same time, sort by pid
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
	int slot = 0; //slot of gantt chart
	int completeProc = 0; //completed process
	std::queue<int> rq; //ready queue
	int currProc = 0; //current process
    
	//Sort by arrival order
	qsort(proc, pn, sizeof(process), compareProc);
    
	while (completeProc < pn) {
		while (currProc < pn && proc[currProc].arrivalTime <= currTime) { //already arrived
			if (!proc[currProc].complete) { //not done
				rq.push(currProc); //push to ready queue
		        }
			currProc++; //next process
		}
        
		if (rq.empty()) { //ready queue is empty and not complete
			if (completeProc < pn) {
				sprintf(ganttChart[slot], "|x|"); //Nothing has arrived yet
				currTime += 1;
				slot += 1;
				totalIdleTime += 1;
				continue;
			}
		}
        
		if (!rq.empty()) { //process in ready queue
			int curr = rq.front(); //select current process(front of ready queue)
			rq.pop();
            
            		//when running for the first time
			if (!proc[curr].start) {
				proc[curr].startTime = currTime;
				proc[curr].start = 1; //start flag
			}
            
			//choose the shorter one between quantum and remaining time
			int executeTime = std::min(quantum, (int)proc[curr].remainingTime);
			for (int i = 0; i < executeTime; i++) { //execute process
 				sprintf(ganttChart[slot], "|P%d|", proc[curr].pid);
				currTime += 1;
				proc[curr].remainingTime -= 1;
				slot += 1;
			}
            
			//End of process execution
			if (proc[curr].remainingTime <= 0) {
				proc[curr].complete = 1;
				proc[curr].completionTime = currTime;
				completeProc++;
			}
			else { //Check processes that arrived while running
				while (currProc < pn && proc[currProc].arrivalTime <= currTime) {
					if (!proc[currProc].complete) { //not done
						rq.push(currProc); //push to ready queue
					}
					currProc++; //next process
				}
				rq.push(curr); //unfinished process, add to ready queue again
			}
			currTime += 0.1; //add context switch time
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
	cpuUtil = 100 * (totalBurstTime / currTime); //totalburst / (totalburst+context switching)
	printf("CPU Utilization = %.2f%%\n", cpuUtil);
	
	return 0;
}

