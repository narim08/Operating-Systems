#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


int FIFO_Algo(int *referStr, int frame, int page) {
	int fifoFault = 0;
	int currPage = -1;
	int rear = 0; //queue tail
	
	// ========= Initialize frame array ========= //
	int frameArr[frame];
	for (int i=0; i<frame; i++) {
		frameArr[i] = -1;
	}
	
	// ============ Page replacement ============ //
	for (int i=0; i<page; i++) {
		currPage = referStr[i]; //current page number
		bool pageExist = false;
		
		// Check page existence
		for (int j=0; j<frame; j++) {
			if (currPage == frameArr[j]) {
				pageExist = true;
				break;
			}
		}
		
		// Page fault
		if (!pageExist) {
			fifoFault++;
			// Circular Queue
			frameArr[rear] = currPage;
			rear = (rear+1) % frame;
		}
	}
	return fifoFault;
}


int main(int argc, char*argv[]) {
	int referStr[1000]; //reference string
	int frame = 0; //number of page frames
	int page = 0; // number of pages
	
	// ============= Read input file ============= //
	FILE* fp = fopen(argv[1], "r");
	if(!fp) {
		perror("file open error!"); exit(1);
	}

	fscanf(fp, "%d", &frame);
	if(frame > 1000) {
		perror("page frame maximum is 1000!"); exit(1);
	}
	
	while(fscanf(fp, "%d", &referStr[page]) != EOF) {
		page++;
	}
	fclose(fp);
	
	// ============ Execute algorithem ============ //
	//int optimalFault = Optimal_Algo(referStr, frame, page);
	int fifoFault = FIFO_Algo(referStr, frame, page);
	//int lruFault = LRU_algo(referStr, frame, page);
	//int clockFault = Clock_algo(referStr, frame, page);
	
	
	// =============== Print result =============== //
	/*printf("Optimal Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)optimalFault/page)*100);
	*/
	printf("FIFO Algorithm:\n");
	printf("Number of Page Faults: %d\n", fifoFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)fifoFault/page)*100);
	/*
	printf("LRU Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)lruFault/page)*100);
	
	printf("Clock Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)clockFault/page)*100);
	*/

	return 0;
}
