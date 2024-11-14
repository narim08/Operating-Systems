#include <stdio.h>
#include <stdlib.h>
#include <string.h>





int main(int argc, char*argv[]) {
	int referStr[1000]; //reference string
	int frame = 0; //number of page frames
	int page = 0; // number of pages
	
	// ============= Read input file ============= //
	FILE* fp = fopen(argv[1], "r");
	if(!fp) {
		perror("file open error!"); exit(1);
	}

	fscanf(fp, "%d", &frameCnt);
	if(frame > 1000) {
		perror("page frame maximum is 1000!"); exit(1);
	}
	
	while(fscanf(fp, "%d", &referStr[page]) != EOF) {
		page++;
	}
	fclose(fp);
	
	
	// ============ Execute algorithem ============ //
	int optimalFault = Optimal_Algo(referStr, frame, page);
	int fifoFault = FIFO_Algo(referStr, frame, page);
	int lruFault = LRU_algo(referStr, frame, page);
	int clockFault = Clock_algo(referStr, frame, page);
	
	
	// =============== Print result =============== //
	printf("Optimal Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)optimalFault/page)*100);
	
	printf("FIFO Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)optimalFault/page)*100);
	
	printf("LRU Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)optimalFault/page)*100);
	
	printf("Clock Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n", ((float)optimalFault/page)*100);
	

	return 0;
}
