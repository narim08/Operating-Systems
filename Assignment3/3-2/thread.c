#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_PROCESSES 128

int res = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* thread function */
void *threadFunc_Sum(void *arg) {
    FILE *file = (FILE *)arg;
    int num1, num2;
    
    if (fscanf(file, "%d\n%d\n", &num1, &num2) == 2) {
    	//read and add two numbers
        int sum = num1 + num2;
        pthread_mutex_lock(&mutex);
        res += sum;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    struct timespec start, end;
    pthread_t threads[MAX_PROCESSES];

    FILE *file = fopen("./temp.txt", "r");
    if (file == NULL) {
        perror("file open error!");
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start); //start measuring performance time
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pthread_create(&threads[i], NULL, threadFunc_Sum, (void *)file) != 0) {
            perror("create thread error!");
            return -1;
        }
    }

    for (int i = 0; i < MAX_PROCESSES; i++) { //parent thread
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end); //end of performance time measurement
    
    fclose(file);
    
    double resTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("value of thread : %d\n", res);
    printf("%.6f\n", resTime);

    return 0;
}
