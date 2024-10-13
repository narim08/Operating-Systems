#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 64

int main() {
    FILE *f_write = fopen("./temp.txt", "w");
    if (f_write == NULL) {
        perror("file open error!\n");
        return -1;
    }

    //write i value twice as much as the MAX_PROCESS
    for (int i = 0; i < MAX_PROCESSES * 2; i++) {
        fprintf(f_write, "%d\n", i + 1);
    }

    fclose(f_write);
    
    return 0;
}
