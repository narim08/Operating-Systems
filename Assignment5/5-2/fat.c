#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FILE 100
#define NUM_DATA_BLOCK 1024
#define BLOCK_SIZE 32
#define MAX_FILE_NAME 100

typedef struct {
	char fileName[MAX_FILE_NAME];
	int fileSize;
	int firstBlock; //starting block in the FAT table (start sector)
	bool use; //use of free
} file;
file fileEntry[MAX_FILE];
int fatTable[NUM_DATA_BLOCK]; //File Allocation Table
char dataBlock[NUM_DATA_BLOCK][BLOCK_SIZE]; //actual storage location where file data is written
int blockOffset[NUM_DATA_BLOCK] = {0};


//Restores the state of the file system
void load_file_system() {
	FILE *fp = fopen("fatLoad.bin", "rb");
	if (!fp) { 	//initialize file system
		printf("Warning: No saved state found. Starting fresh.\n");
		
		for (int i=0; i<NUM_DATA_BLOCK; i++) {
			fatTable[i]=0;
		}
		for (int i=0; i<MAX_FILE; i++) {
			fileEntry[i].fileName[0]='\0';
			fileEntry[i].fileSize = 0;
			fileEntry[i].firstBlock = 0;
			fileEntry[i].use = false; //free
		}
		return;
	}
	else { //load (FAT table, file entries, data)
		fread(fatTable, sizeof(int), NUM_DATA_BLOCK, fp);
		fread(fileEntry, sizeof(file), MAX_FILE, fp);
		fread(dataBlock, BLOCK_SIZE, NUM_DATA_BLOCK, fp);
		fread(blockOffset, sizeof(int), NUM_DATA_BLOCK, fp);
	}
	fclose(fp);
}


//Creates a file and allocates a block
void create_file(const char *name) {
	//Check if file exists
	for (int i=0; i<MAX_FILE; i++) {
		if (fileEntry[i].use && strcmp(fileEntry[i].fileName, name)==0) {
			printf("file exist!\n"); exit(1);
		}
	}
	
	//Find file entries that are free
	int fileIndex = -1;
	for (int i=0; i<MAX_FILE; i++) {
		if (!fileEntry[i].use) { //free file
			fileIndex = i;
			break;
		}
	}
	if (fileIndex == -1) {
		printf("All file entries are full!\n"); exit(1);
	}
	
	//Find first block(fatTable) that are free
	int blockIndex = -1;
	for (int i=0; i<NUM_DATA_BLOCK; i++) {
		if (fatTable[i] == 0) {
			blockIndex = i;
			break;
		}
	}
	if (blockIndex == -1) {
		printf("All data block(FAT table) are full!\n"); exit(1);
	}
	
	//Create file entry
	strcpy(fileEntry[fileIndex].fileName, name);
	fileEntry[fileIndex].fileSize = 0;
	fileEntry[fileIndex].firstBlock = blockIndex;
	fileEntry[fileIndex].use = true;
	
	fatTable[blockIndex] = -1; //first block of a file is marked
	
	printf("File '%s' created.\n", name);
	
	return;
}


//Writes data to a file, allocating and linking blocks in the FAT table
void write_file(const char *name, const char *content) {
	
	//Find file
	int fileIndex = -1;
	for (int i=0; i<MAX_FILE; i++) {
		if (fileEntry[i].use && strcmp(fileEntry[i].fileName, name)==0) {
			fileIndex = i;
			break;
		}
	}
	if (fileIndex == -1) {
		printf("File not found!\n"); exit(1);
	}
	
	const char *contentPtr = content;
	int currBlock = fileEntry[fileIndex].firstBlock;
	
	//Find the last block of the current file
	while (fatTable[currBlock] != -1) {
		currBlock = fatTable[currBlock];
	}
	
	//Continue writing from the last block
	while (*contentPtr) {
		if (currBlock == -1) return;
		
		int remainingSpace = BLOCK_SIZE - blockOffset[currBlock];
		int writeSize = strlen(contentPtr) < remainingSpace ? strlen(contentPtr) : remainingSpace;
		
		memcpy(dataBlock[currBlock]+blockOffset[currBlock], contentPtr, writeSize);
		contentPtr += writeSize; //move start address of content
		blockOffset[currBlock] += writeSize;
		
		if (blockOffset[currBlock] == BLOCK_SIZE) {
			int nextBlock = -1;
			for (int i=0; i<NUM_DATA_BLOCK; i++) {
				if (fatTable[i] == 0) {
					nextBlock = i;
					break;
				}
			}
			if (nextBlock == -1) {
				printf("All data block(FAT table) are full!\n"); exit(1);
			}
			
			fatTable[currBlock] = nextBlock;
			fatTable[nextBlock] = -1;
			currBlock = nextBlock;
		}
	}
	fileEntry[fileIndex].fileSize += strlen(content);
	printf("Data written to '%s'.\n", name);
	return;
}


//Reads data from the file, following the linked blocks
void read_file(const char *name) {
	
	//Find file
	int fileIndex = -1;
	for (int i=0; i<MAX_FILE; i++) {
		if (strcmp(fileEntry[i].fileName, name)==0) {
			fileIndex = i;
			break;
		}
	}
	if (fileIndex == -1) {
		printf("File not found!\n"); exit(1);
	}
	
	printf("Content of '%s': ", name);
	
	int currBlock = fileEntry[fileIndex].firstBlock;
	int readLen = 0;
	int totalSize = fileEntry[fileIndex].fileSize;
	
	while(currBlock != -1) {
		for (int i=0; i<BLOCK_SIZE; i++) {
			if (readLen < totalSize) {
				putchar(dataBlock[currBlock][i]);
				readLen++;
			}
		}
		currBlock = fatTable[currBlock];
	}
	printf("\n");
	printf("FAT Table: \n");
	for (int i = 0; i < 10; i++) {
	    printf("Block %d -> %d\n", i, fatTable[i]);
	}

	return;
}


//Deletes the file and releases its blocks in the FAT table
void delete_file(const char *name) {
	
	//Find file
	int fileIndex = -1;
	for (int i=0; i<MAX_FILE; i++) {
		if (strcmp(fileEntry[i].fileName, name)==0) {
			fileIndex = i;
			break;
		}
	}
	if (fileIndex == -1) {
		printf("File not found!\n"); exit(1);
	}
	
	
	int currBlock = fileEntry[fileIndex].firstBlock;
	
	while (currBlock != -1) {
		int nextblock = fatTable[currBlock];
		fatTable[currBlock] = -1;
		currBlock = nextblock;
	}
	strcpy(fileEntry[fileIndex].fileName, "-");
	fileEntry[fileIndex].fileSize = 0;
	fileEntry[fileIndex].use = false;
	
	printf("File '%s' deleted.\n", name);
	
	return;
}


//Displays a list of all files in the system
void list_files() {
	printf("Files in the file system:\n");
	
	for (int i=0; i<MAX_FILE; i++) {
		if (fileEntry[i].use) { //file exist
			printf("File: %s, Size: %d bytes\n", fileEntry[i].fileName, fileEntry[i].fileSize);
		}
	}
	return;
}


//Saves the state of the file system
void save_file_system() {
	FILE *fp = fopen("fatLoad.bin", "wb");
	if (!fp) { 	//initialize file system
		printf("File does not exist!\n"); exit(1);
	}
	else { //save (FAT table, file entries, data)
		fwrite(fatTable, sizeof(int), NUM_DATA_BLOCK, fp);
		fwrite(fileEntry, sizeof(file), MAX_FILE, fp);
		fwrite(dataBlock, BLOCK_SIZE, NUM_DATA_BLOCK, fp);
		fwrite(blockOffset, sizeof(int), NUM_DATA_BLOCK, fp);
	}
	fclose(fp);
	return;
}


int main(int argc, char *argv[]) {
	
	load_file_system(); //restores the state of the file system
	
	if (strcmp(argv[1], "create")==0) {
		create_file(argv[2]);
	}
	else if (strcmp(argv[1], "list")==0) {
		list_files();
	}
	else if (strcmp(argv[1], "write")==0) {
		write_file(argv[2], argv[3]);
	}
	else if (strcmp(argv[1], "read")==0) {
		read_file(argv[2]);
	}
	else if (strcmp(argv[1], "delete")==0) {
		delete_file(argv[2]);
	}
	else {
		printf("invalid command\n"); exit(1);
	}
	
	save_file_system(); //saves the state of the file system

	return 0;
}

