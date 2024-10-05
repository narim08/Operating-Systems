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

void *real_openat;
void *real_read;
void *real_write;
void *real_lseek;
void *real_close;


static asmlinkage long ftrace_openat(const struct pt_regs *regs) {
    open_count++;
    /*
    if (current->pid == target_pid) {
        open_count++;
    }
    */
    return ((asmlinkage int (*)(const struct pt_regs *))real_openat)(regs); 
}


static asmlinkage long ftrace_read(const struct pt_regs *regs) {
	ssize_t bytes = regs->dx;  
	printk(KERN_INFO "ftrace_read called: bytes=%zd\n", bytes);

	read_count++;  
        read_bytes += bytes;  
        /*
    if (current->pid == target_pid) {
        read_count++;  
        read_bytes += bytes;  
    }
    */
    return ((asmlinkage int (*)(const struct pt_regs *))real_read)(regs);  
}


static asmlinkage long ftrace_write(const struct pt_regs *regs) {
	ssize_t bytes = regs->dx; 
	printk(KERN_INFO "ftrace_write called: bytes=%zd\n", bytes);

	write_count++; 
        write_bytes += bytes;
        /*
    if (current->pid == target_pid) {
        write_count++; 
        write_bytes += bytes;
    }*/
    return ((asmlinkage int (*)(const struct pt_regs *))real_write)(regs);  
}


static asmlinkage long ftrace_lseek(const struct pt_regs *regs) {
    lseek_count++; 
    /*
    if (current->pid == target_pid) {
        lseek_count++; 
    }*/
    return ((asmlinkage int (*)(const struct pt_regs *))real_lseek)(regs); 
}


static asmlinkage long ftrace_close(const struct pt_regs *regs) {
    close_count++;  
    /*
    if (current->pid == target_pid) {
        close_count++;  
    }*/
    return ((asmlinkage int (*)(const struct pt_regs *))real_close)(regs);  
}


void printInfo(void) {
	printk(KERN_INFO "[2022202065] a.out file[abc.txt] stats [x] read -%zu / written - %zu\n", read_bytes, write_bytes);
    	printk(KERN_INFO "open[%d], close[%d], read[%d], write[%d], lseek[%d]\n", open_count, close_count, read_count, write_count, lseek_count);
}
EXPORT_SYMBOL(printInfo);


void make_rw(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    if (!pte) {
        printk(KERN_ERR "Invalid PTE\n");
        return;
    }
    
    if (pte->pte & ~_PAGE_RW)
        pte->pte |= _PAGE_RW;
}


void make_ro(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    
    if (!pte) {
        printk(KERN_ERR "Invalid PTE\n");
        return;
    }

    pte->pte = pte->pte & ~_PAGE_RW;
}


static int __init iotracehooking_init(void) {
    // Find system call table
    syscall_table = (void **)kallsyms_lookup_name("sys_call_table");
    
    if (!syscall_table) {
        printk(KERN_ERR "Failed to find syscall table\n");
        return -1;
    }
    
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

    return 0;
}

/* Called when the module exits. */
static void __exit iotracehooking_exit(void) {
	// System call restoration
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


