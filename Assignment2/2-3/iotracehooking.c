#include <linux/module.h>
#include <linux/kallsyms.h>  // kallsyms_lookup_name()
#include <linux/syscalls.h>   
#include <linux/sched.h>      
#include <linux/fdtable.h>    
#include <linux/fs.h> 
#include <linux/dcache.h>
#include "ftracehooking.h"      

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Iotracehooking Module");
MODULE_VERSION("1.0");

void **syscall_table;

int open_count;
int read_count;
size_t read_bytes;
int write_count;
size_t write_bytes; 
int lseek_count;
int close_count;
pid_t callPid = -1; // trace target pid

/* Pointer to store address of the file system call */
asmlinkage long (*real_openat)(const struct pt_regs *);
asmlinkage long (*real_read)(const struct pt_regs *);
asmlinkage long (*real_write)(const struct pt_regs *);
asmlinkage long (*real_lseek)(const struct pt_regs *);
asmlinkage long (*real_close)(const struct pt_regs *);


/* set trace target pid (ftracehooking.c -> iotracehooking.c) */
void setPid(pid_t pid) {
    callPid = pid;
}
EXPORT_SYMBOL(setPid);

/* openat -> ftrace_openat system call hijack */
static asmlinkage long ftrace_openat(const struct pt_regs *regs) {
    pid_t pid = current->pid;
    
    if (pid == callPid) {
    	open_count++;
    }
    return real_openat(regs); 
}

/* read -> ftrace_read system call hijack */
static asmlinkage long ftrace_read(const struct pt_regs *regs) {
    pid_t pid = current->pid;
    
    if (pid == callPid) {
    	ssize_t bytes = regs->dx;  
    	read_count++;  
    	read_bytes += bytes;
    }  
    return real_read(regs);  
}

/* write -> ftrace_write system call hijack */
static asmlinkage long ftrace_write(const struct pt_regs *regs) {
    pid_t pid = current->pid;
    
    if (pid == callPid) {
    	ssize_t bytes = regs->dx; 
    	write_count++; 
    	write_bytes += bytes;
    }
    return real_write(regs);  
}

/* lseek -> ftrace_lseek system call hijack */
static asmlinkage long ftrace_lseek(const struct pt_regs *regs) {
    pid_t pid = current->pid;
    
    if (pid == callPid) {
    	lseek_count++;
    } 
    return real_lseek(regs); 
}

/* close -> ftrace_close system call hijack */
static asmlinkage long ftrace_close(const struct pt_regs *regs) {
    pid_t pid = current->pid;
    
    if (pid == callPid) {
    	close_count++;  
    }    
    return real_close(regs);  
}

/* send trace result to ftracehooking.c */
void printInfo(void) {
	printk(KERN_INFO "[2022202065] a.out file[abc.txt] stats [x] read - %zu / written - %zu\n", read_bytes, write_bytes);
    	printk(KERN_INFO "open[%d], close[%d], read[%d], write[%d], lseek[%d]\n", open_count, close_count, read_count, write_count, lseek_count);
}
EXPORT_SYMBOL(printInfo); 


/* Grant write permission to system call table */
void make_rw(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    if (pte->pte & ~_PAGE_RW)
        pte->pte |= _PAGE_RW;
}

/* Reclaim write permission to system call table */
void make_ro(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    pte->pte = pte->pte & ~_PAGE_RW;
}

/* Called when loading module */
static int __init iotracehooking_init(void) {
    // Find system call table
    syscall_table = (void **)kallsyms_lookup_name("sys_call_table");
    
    // Change permission of the page of system call table(read, write)
    make_rw(syscall_table);
    
    real_openat = syscall_table[__NR_openat];
    real_read = syscall_table[__NR_read];
    real_write = syscall_table[__NR_write];
    real_lseek = syscall_table[__NR_lseek];
    real_close = syscall_table[__NR_close];

    syscall_table[__NR_openat] = ftrace_openat;
    syscall_table[__NR_read] = ftrace_read;
    syscall_table[__NR_write] = ftrace_write;
    syscall_table[__NR_lseek] = ftrace_lseek;
    syscall_table[__NR_close] = ftrace_close;

    make_ro(syscall_table);

    return 0;
}

/* Called when the module exits. */
static void __exit iotracehooking_exit(void) {
    // System call restoration
    make_rw(syscall_table);
    syscall_table[__NR_openat] = real_openat;
    syscall_table[__NR_read] = real_read;
    syscall_table[__NR_write] = real_write;
    syscall_table[__NR_lseek] = real_lseek;
    syscall_table[__NR_close] = real_close;
        
    // Recover the page's permission(read-only)
    make_ro(syscall_table);
    
}

module_init(iotracehooking_init);
module_exit(iotracehooking_exit);


