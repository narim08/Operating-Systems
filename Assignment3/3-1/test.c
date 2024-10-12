#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main()
{
	syscall(336, 1);
	
	return 0;
}
