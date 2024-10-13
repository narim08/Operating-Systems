#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Process_tracer Module");
MODULE_VERSION("1.0");

void **syscall_table;
void *real_os_ftrace; // Pointer to store the address of the existing os_ftrace system.


/* Information on task status */
static char *stateInfo(struct task_struct *task) {
    switch (task->state) {
        case TASK_RUNNING:
            return "Running or ready";
        case TASK_UNINTERRUPTIBLE:
            return "Wait with ignoring all signals";
        case TASK_INTERRUPTIBLE:
            return "Wait";
        case __TASK_STOPPED:
            return "Stopped";
        case EXIT_ZOMBIE:
            return "Zombie process";
        case EXIT_DEAD:
            return "Dead";
        default:
            return "etc.";
    }
}


/* os_ftrace -> process_tracer system call hijack */
asmlinkage pid_t process_tracer(const struct pt_regs *regs) {
    pid_t trace_task = (pid_t)regs->di; //get pid
    struct task_struct *task;
    struct task_struct *parent_task;
    
    struct list_head *list; //task linked list
    int sibling_count = 0;
    int child_count = 0;

    task = pid_task(find_vpid(trace_task), PIDTYPE_PID); //Find task by pid
    if (!task) { //can't find it
        return -1;
    }


    /* Information output from the process */
    printk(KERN_INFO "##### TASK INFORMATION of ''[%d] %s'' #####\n",trace_task, task->comm); //(1) process name

    printk(KERN_INFO "- task state : %s\n", stateInfo(task)); //(2) current process state

    printk(KERN_INFO "- Process Group Leader : [%d] %s\n", 
           task_pgrp_nr(task), task->group_leader->comm); //(3) process group information

    printk(KERN_INFO "- Number of context switches : %lu\n", 
           task->nvcsw + task->nivcsw); //(4) number of context switches

    printk(KERN_INFO "- Number of calling fork() : %d\n", task->fork_count); //(5) Number of times fork() was called

    //(6) parent process information
    parent_task = task->real_parent;
    printk(KERN_INFO "- it's parent process : [%d] %s\n", 
           parent_task->pid, parent_task->comm);

    //(7) sibling processes information
    printk(KERN_INFO "- it's sibling process(es) :\n");
    list_for_each(list, &parent_task->children) {
        struct task_struct *sibling = list_entry(list, struct task_struct, sibling);
        if (sibling->pid != task->pid) {
            printk(KERN_INFO "    > [%d] %s\n", sibling->pid, sibling->comm);
            sibling_count++;
        }
    }
    if (sibling_count == 0) {
    	printk(KERN_INFO "    > It has no sibling.\n");
    }
    else {
    	printk(KERN_INFO "    > This process has %d sibling process(es)\n", sibling_count);	
    }

    //(8) child processes information
    printk(KERN_INFO "- it's child process(es) :\n");
    list_for_each(list, &task->children) {
        struct task_struct *child = list_entry(list, struct task_struct, sibling);
        printk(KERN_INFO "    > [%d] %s\n", child->pid, child->comm);
        child_count++;
    }
    if (child_count == 0) {
    	printk(KERN_INFO "    > It has no child.\n");
    }
    else {
    	printk(KERN_INFO "    > This process has %d child process(es)\n", child_count);
    }
    
    
    // end print
    printk(KERN_INFO "##### END OF INFORMATION #####\n");

    return trace_task; //Returns the entered pid
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
static int __init process_tracer_init(void)
{
	// Find system call table
	syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
	
	// Change permission of the page of system call table(read, write)
	make_rw(syscall_table);
	real_os_ftrace = syscall_table[__NR_os_ftrace];
	syscall_table[__NR_os_ftrace] = process_tracer;

	return 0;
}

/* Called when the module exits. */
static void __exit process_tracer_exit(void)
{
	// System call restoration
	syscall_table[__NR_os_ftrace] = real_os_ftrace;

	// Recover the page's permission(read-only)
	make_ro(syscall_table);
}


module_init(process_tracer_init);
module_exit(process_tracer_exit);

