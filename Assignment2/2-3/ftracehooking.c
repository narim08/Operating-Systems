#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>	// kallsyms_lookup_name()
#include <linux/syscalls.h>	// __SYSCALL_DEFINEx()
#include <asm/syscall_wrapper.h>// __SYSCALL_DEFINEx()
#include <asm/ptrace.h>
#include <linux/sched.h>
#include "ftracehooking.h"	


void **syscall_table;
void *real_os_ftrace; // Pointer to store the address of the existing os_ftrace system.
pid_t target_pid = -1;

struct ftrace_data ftrace_info = {0};
EXPORT_SYMBOL(ftrace_info);

//extern struct ftrace_data ftrace_info;

static char* get_process_name(void) {
    return current->comm;  // 현재 실행 중인 프로세스의 이름 반환
}

static asmlinkage int my_ftrace(const struct pt_regs *regs) {
    pid_t pid = (pid_t)regs->di;  // first argument

    if (pid > 0) {
        printk(KERN_INFO "OS Assignment 2 ftrace [%d] Start\n", pid);
        target_pid = pid;
    } else if (pid == 0) {
    	printk(KERN_INFO "OS Assignment 2 ftrace [%d] End\n", pid);
    	printk(KERN_INFO "[2022202065] %s file[%s] stats [x] read -%zu / written - %zu\n", get_process_name(), ftrace_info.last_file_name, ftrace_info.total_read_bytes, ftrace_info.total_write_bytes);
    	printk(KERN_INFO "open[%d], close[%d], read[%d] (%zu bytes), write[%d] (%zu bytes), lseek[%d]\n", ftrace_info.open_count, ftrace_info.close_count, ftrace_info.read_count, ftrace_info.total_read_bytes, ftrace_info.write_count, ftrace_info.total_write_bytes, ftrace_info.lseek_count);
        
    }

    // return sys_os_ftrace
    return ((asmlinkage int (*)(const struct pt_regs *))real_os_ftrace)(regs);
}

EXPORT_SYMBOL(target_pid);

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
	syscall_table[__NR_os_ftrace] = (void *)my_ftrace;

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
MODULE_LICENSE("GPL");
