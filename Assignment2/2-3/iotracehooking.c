#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include "ftracehooking.h"

void **syscall_table;
void *real_openat;
void *real_read;
void *real_write;
void *real_lseek;
void *real_close;


static asmlinkage long ftrace_openat(const struct pt_regs *regs) {
    printk(KERN_INFO "ftrace_openat() called!\n");

    return ((asmlinkage long (*)(int, const char __user *, int, umode_t))real_openat)(regs->di, (const char __user *)regs->si, regs->dx, regs->r10);
}

static asmlinkage long ftrace_read(const struct pt_regs *regs) {
    printk(KERN_INFO "ftrace_read() called!\n");
   
    return ((asmlinkage long (*)(unsigned int, char __user *, size_t))real_read)(regs->di, (char __user *)regs->si, regs->dx);
}

static asmlinkage long ftrace_write(const struct pt_regs *regs) {
    printk(KERN_INFO "ftrace_write() called!\n");
    
    return ((asmlinkage long (*)(unsigned int, const char __user *, size_t))real_write)(regs->di, (const char __user *)regs->si, regs->dx);
}

static asmlinkage long ftrace_lseek(const struct pt_regs *regs) {
    printk(KERN_INFO "ftrace_lseek() called!\n");
    
    return ((asmlinkage long (*)(unsigned int, off_t, unsigned int))real_lseek)(regs->di, regs->si, regs->dx);
}

static asmlinkage long ftrace_close(const struct pt_regs *regs) {
    printk(KERN_INFO "ftrace_close() called!\n");
    
    return ((asmlinkage long (*)(unsigned int))real_close)(regs->di);
}


/* Grant write permission to system call table */
void make_rw(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
}
