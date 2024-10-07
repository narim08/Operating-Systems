#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>	// kallsyms_lookup_name()
#include <linux/syscalls.h>	// __SYSCALL_DEFINEx()
#include <asm/syscall_wrapper.h>// __SYSCALL_DEFINEx()
#include <asm/ptrace.h>
#include <linux/sched.h>
#include "ftracehooking.h"	

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ftracehooking Module");
MODULE_VERSION("1.0");


void **syscall_table;
void *real_os_ftrace; // Pointer to store the address of the existing os_ftrace system.

pid_t target_pid = -1;

extern void printInfo(void);
extern void setPid(pid_t pid);


static asmlinkage int my_ftrace(const struct pt_regs *regs) {
    pid_t pid = (pid_t)regs->di;  // first argument

    if (pid > 0) {
        printk(KERN_INFO "OS Assignment 2 ftrace [%d] Start\n", pid);
        target_pid = pid;
        setPid(target_pid);
    } else if (pid == 0) {
    	printInfo();
    	printk(KERN_INFO "OS Assignment 2 ftrace [%d] End\n", target_pid);
    }

    // return sys_os_ftrace
    return ((asmlinkage int (*)(const struct pt_regs *))real_os_ftrace)(regs);
}

/* Grant write permission to system call table */
void make_rw(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
}

/* Reclaim write permission to system call table */
void make_ro(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	pte->pte = pte->pte &~ _PAGE_RW;
}

/* Called when loading module */
static int __init hooking_init(void)
{
	// Find system call table
	syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
	
	// Change permission of the page of system call table(read, write)
	make_rw(syscall_table);
	real_os_ftrace = syscall_table[__NR_os_ftrace];
	syscall_table[__NR_os_ftrace] = my_ftrace;

	return 0;
}

/* Called when the module exits. */
static void __exit hooking_exit(void)
{
	// System call restoration
	syscall_table[__NR_os_ftrace] = real_os_ftrace;

	// Recover the page's permission(read-only)
	make_ro(syscall_table);
}


module_init(hooking_init);
module_exit(hooking_exit);

