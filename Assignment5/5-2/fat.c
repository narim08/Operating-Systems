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


//Restores the state of the file system
void load_file_system() {
	FILE *fp = fopen("fatLoad.bin", "rb");
	if (!fp) { 	//initialize file system
		printf("Warning: No saved state found. Starting fresh.");
		
		for (int i=0; i<NUM_DATA_BLOCK; i++) {
			fatTable[i]=-2;
		}
		for (int i=0; i<MAX_FILE; i++) {
			fileEntry[i].use = false; //free
		}
		return;
	}
	else { //load FAT table, file entries, data)
		fread(fatTable, sizeof(int), NUM_DATA_BLOCK, fp);
		fread(fileEntry, sizeof(file), MAX_FILE, fp);
		fread(dataBlock, BLOCK_SIZE, NUM_DATA_BLOCK, fp);
	}
	fclose(fp);
}


//Creates a file and allocates a block
void create_file(const char *name) {
	//Check if file exists
	for (int i=0; i<MAX_FILE; i++) {
		if (strcmp(fileEntry[i].fileName, name)==0) {
			perror("file exist!"); exit(1);
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
		perror("All file entries are full!"); exit(1);
	}
	
	//Find first block(fatTable) that are free
	int blockIndex = -1;
	for (int i=0; i<NUM_DATA_BLOCK; i++) {
		if (fatTable[i] == -2) {
			blockIndex = i;
			break;
		}
	}
	if (blockIndex == -1) {
		perror("All data block(FAT table) are full!"); exit(1);
	}
	
	//Create file entry
	strcpy(fileEntry[fileIndex].fileName, name);
	fileEntry[fileIndex].fileSize = 0;
	fileEntry[fileIndex].firstBlock = blockIndex;
	fileEntry[fileIndex].use = true;
	
	fatTable[block_index] = -1; //the last block of a file is marked with -1
	
	printf("File '%s' created.\n", name);
	
	return;
}


//Writes data to a file, allocating and linking blocks in the FAT table
void write_file(const char *name, const char *content) {
	
	//Find file
	int fileIndex = -1;
	for (int i=0; i<MAX_FILE; i++) {
		if (strcmp(fileEntry[i].fileName, name)==0) {
			fileIndex = i;
			break;
		}
	}
	if (fileIndex == -1) {
		perror("File not found!"); exit(1);
	}
	
	
	int currBlock = fileEntry[fileIndex].firstBlock;
	int contentLen = strlen(content);
	int remainingLen = contentLen;
	int writeLen = 0;
	
	
	while(writeLen < contentLen) {
		
	}
	

	return;
}


//Reads data from the file, following the linked blocks
void read_file(const char *name) {
}


//Deletes the file and releases its blocks in the FAT table
void delete_file(const char *name) {
}


//Displays a list of all files in the system
void list_files() {
}


//Saves the state of the file system
void save_file_system() {
	
	
	return;
}


int main(int argc, char *argv[]) {
	
	load_file_system(); //restores the state of the file system
	
	if (strcmp(argv[1]=="create")==0) {
		create_file(argv[2]);
	}
	else if (strcmp(argv[1]=="list")==0) {
		list_file();
	}
	else if (strcmp(argv[1]=="write") {
		write_file(argv[2], argv[3]);
	}
	else if (strcmp(argv[1]=="read")==0) {
		read_file(argv[2]);
	}
	else if (strcmp(argv[1]=="delete")==0) {
		delete_file(argv[2]);
	}
	else {
		perror("invalid command"); exit(1);
	}
	
	save_file_system(); //saves the state of the file system

	return 0;
}

