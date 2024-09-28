#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(void)
{
	int ret;
	pid_t pid = getpid();
	printf("pid: %d\n", pid);
	ret = syscall(336, pid);
	printf("%d\n", ret);
	return 0;
}
