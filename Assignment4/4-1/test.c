#include <unistd.h>
#include <sys/types.h>
#include <linux/unistd.h>

#define __NR_os_ftrace 336

int main(void) {
	syscall(__NR_os_ftrace, getpid());
	return 0;
}
