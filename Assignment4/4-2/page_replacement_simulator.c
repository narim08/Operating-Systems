#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


//Optimal: Replaces the page that will not be used for the longest period in the future
int Optimal_Algo(int *referStr, int frame, int page) {
	int optimalFault = 0;
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
				/*
				printf("Page hit   %d: ", currPage);
            			for (int k = 0; k < frame; k++) {
               			printf("%d ", frameArr[k]);
            			}
            			printf("\n");
            			*/
				break;
			}
		}
		
		// Page fault
		if (!pageExist) {
			optimalFault++;
			
			// empty frame
			if (rear < frame) {
				frameArr[rear++] = currPage;
			}	
			else { // Optimal Policy
				int replacePage = 0; // Page number to be replaced
				int furthestPage = i+1; // furthest page number
				
				for (int j=0; j<frame; j++) { // j==frame
					int next = 0; // Next Reference Position Distance
					for (next=i+1; next<page; next++) {
						if (frameArr[j]==referStr[next])
							break;
					}
					
					if (next == page) { // Not future referenced
						replacePage = j;
						break;
					}
					
					if (next > furthestPage) { // update to farthest
						furthestPage = next;
						replacePage = j;
					}
				}
				frameArr[replacePage] = currPage;
			}/*
			printf("Page Fault %d: ", currPage);
            		for (int k = 0; k < frame; k++) {
               		printf("%d ", frameArr[k]);
            		}
            		printf("\n");*/
		}	
	}
	return optimalFault;
}


//FIFO(First In, First Out): Replaces the oldest page that was loaded into memory
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
				/*
				printf("Page hit   %d: ", currPage);
            			for (int k = 0; k < frame; k++) {
               			printf("%d ", frameArr[k]);
            			}
            			printf("\n");
            			*/
				break;
			}
		}
		
		// Page fault
		if (!pageExist) {
			fifoFault++;
			// Circular Queue
			frameArr[rear] = currPage;
			rear = (rear+1) % frame;
			/*
			printf("Page Fault %d: ", currPage);
            		for (int k = 0; k < frame; k++) {
               		printf("%d ", frameArr[k]);
            		}
            		printf("\n");*/
		}
	}
	return fifoFault;
}


//LRU(Least Recently Used): Replaces the page that has not been used for the longest period
int LRU_algo(int *referStr, int frame, int page) {
	int lruFault = 0;
	int currPage = -1;
	int hitPage = -1;
	int top = 0; //stack tail
	
	// ========= Initialize frame array ========= //
	int frameArr[frame];
	for (int i=0; i<frame; i++) {
		frameArr[i] = -1;
	}
	
	// ============ Page replacement ============ //
	for (int i=0; i<page; i++) {
		currPage = referStr[i]; // current page number
		bool pageExist = false;
		
		// Check page existence
		for (int j=0; j<frame; j++) {
			if (currPage == frameArr[j]) {
				pageExist = true;
				hitPage = j;
				break;
			}
		}
		
		// Page fault
		if (!pageExist) {
			lruFault++;
			
			// empty frame
			if (top < frame) {
				frameArr[top++] = currPage;
			}
			else { // full frame
				for (int j=0; j<top-1; j++) {
					frameArr[j] = frameArr[j+1];
				}
				frameArr[top-1] = currPage;
			}
			/*
			printf("Page Fault %d: ", currPage);
            		for (int k = 0; k < frame; k++) {
               		printf("%d ", frameArr[k]);
            		}
            		printf("\n");
            		*/
		}
		else { // currPage hit
			for (int j=hitPage; j<top-1; j++) {
				frameArr[j] = frameArr[j+1];
			}			
			frameArr[top-1] = currPage;
			/*
			printf("Page hit   %d: ", currPage);
            		for (int k = 0; k < frame; k++) {
               		printf("%d ", frameArr[k]);
            		}
            		printf("\n");*/
		}
	}
	return lruFault;
}


//Clock: Circular structure with a reference bit to determine which pages to replace, giving pages a "second chance" before replacing them.
typedef struct {
	int pageNum;
	int useBit;
} Frame;

int Clock_algo(int *referStr, int frame, int page) {
	int clockFault = 0;
	int currPage = -1;
	int hand = 0; //clock hand
	int tail = 0; //frame end
	
	// ========= Initialize frame array ========= //
	Frame frameArr[frame];
	for (int i=0; i<frame; i++) {
		frameArr[i].pageNum = -1;
		frameArr[i].useBit = 0;
	}
	
	// ============ Page replacement ============ //
	for (int i=0; i<page; i++) {
		currPage = referStr[i]; // current page number
		bool pageExist = false;
		
		// Check page existence
		for (int j=0; j<frame; j++) {
			if (currPage == frameArr[j].pageNum) {
				pageExist = true;
				frameArr[j].useBit = 1;
				/*
				printf("Page hit   %d: ", currPage);
            			for (int k = 0; k < frame; k++) {
               			printf("%d ", frameArr[k].pageNum);
            			}
            			printf("\n");
            			*/
            			break;
			}
		}
		
		// Page fault
		if (!pageExist) {
			clockFault++;
			
			// empty frame
			if (tail < frame) {
				frameArr[tail].pageNum = currPage;
				frameArr[tail].useBit = 1;
				tail++;
			}
			else { // full frame
				while(1) { // Find and replace pages with use bit of 0
					if (frameArr[hand].useBit == 1) {
						frameArr[hand].useBit = 0;
						hand = (hand+1) % frame; //circular
					}
					else {
						frameArr[hand].pageNum = currPage;
						frameArr[hand].useBit = 1;
						hand = (hand+1) % frame;
						break;
					}
				}
			}/*
			printf("Page Fault %d: ", currPage);
            		for (int k = 0; k < frame; k++) {
               		printf("%d ", frameArr[k].pageNum);
            		}
            		printf("\n");*/
		}
		
        }
	return clockFault;
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
	int optimalFault = Optimal_Algo(referStr, frame, page);
	int fifoFault = FIFO_Algo(referStr, frame, page);
	int lruFault = LRU_algo(referStr, frame, page);
	int clockFault = Clock_algo(referStr, frame, page);
	
	
	// =============== Print result =============== //
	printf("Optimal Algorithm:\n");
	printf("Number of Page Faults: %d\n", optimalFault);
	printf("Page Fault Rate: %.2f%%\n\n", ((float)optimalFault/page)*100);
	
	printf("FIFO Algorithm:\n");
	printf("Number of Page Faults: %d\n", fifoFault);
	printf("Page Fault Rate: %.2f%%\n\n", ((float)fifoFault/page)*100);
	
	printf("LRU Algorithm:\n");
	printf("Number of Page Faults: %d\n", lruFault);
	printf("Page Fault Rate: %.2f%%\n\n", ((float)lruFault/page)*100);
	
	printf("Clock Algorithm:\n");
	printf("Number of Page Faults: %d\n", clockFault);
	printf("Page Fault Rate: %.2f%%\n\n", ((float)clockFault/page)*100);
	

	return 0;
}
