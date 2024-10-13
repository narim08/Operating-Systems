all: numgen forkTest threadTest

numgen: numgen.c
	gcc -o numgen numgen.c
forkTest: fork.c
	gcc -o forkTest fork.c
threadTest: thread.c
	gcc -o threadTest thread.c -pthread
	
