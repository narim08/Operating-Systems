#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/pid.h>

SYSCALL_DEFINE1(os_ftrace, pid_t, pid)
{
	printk("ORIGINAL ftrace() called! PID is [%d]\n", pid);
	return 0;
}
