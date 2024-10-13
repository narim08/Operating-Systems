#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_PROCESSES 128

int read2Num(FILE *file, int *num1, int *num2) {
    //reads and returns 2 numbers
    return fscanf(file, "%d\n%d\n", num1, num2) == 2;
}

int main() {
    int res = 0;
    int status;
    pid_t pid;
    struct timespec start, end;

    FILE *file = fopen("./temp.txt", "r");
    if (file == NULL) {
        perror("file open error!");
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start); //start measuring performance time
    for (int i = 0; i < MAX_PROCESSES; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork error!");
            return -1;
        }
        if (pid == 0) {  //child process
            int num1, num2;
            if (read2Num(file, &num1, &num2)) {
            	int sum = num1 + num2;
                exit(sum & 0xFF); //esure the exit value is 8 bits //8 bit right shift
            } else {
                exit(0);
            }
        }
    }

    for (int i = 0; i < MAX_PROCESSES; i++) { //parent process
        wait(&status); //wait for child process to terminate
        if (WIFEXITED(status)) {
            res += WEXITSTATUS(status);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end); //End of performance time measurement
    
    fclose(file);

    double resTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("value of fork : %d\n", res);
    printf("%.6f\n", resTime);

    return 0;
}
