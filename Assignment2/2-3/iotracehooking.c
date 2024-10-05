#include <linux/module.h>
#include <linux/kallsyms.h>  // kallsyms_lookup_name()
#include <linux/syscalls.h>   
#include <linux/sched.h>      
#include <linux/fdtable.h>    
#include <linux/fs.h> 
#include <linux/dcache.h>
#include "ftracehooking.h"      

extern pid_t target_pid;

void **syscall_table;

asmlinkage long (*real_openat)(const struct pt_regs *);
asmlinkage long (*real_read)(const struct pt_regs *);
asmlinkage long (*real_write)(const struct pt_regs *);
asmlinkage long (*real_lseek)(const struct pt_regs *);
asmlinkage long (*real_close)(const struct pt_regs *);


static void get_file_name(int fd) {
    struct file *file;
    struct path *path;
    char *buf = (char *)__get_free_page(GFP_KERNEL);  // 페이지 크기 할당

    if (buf) {
        file = fget(fd);  // fd에 대한 파일 구조체 얻기
        if (file) {
            path = &file->f_path;
            path_get(path);
            strncpy(ftrace_info.last_file_name, d_path(path, buf, PAGE_SIZE), NAME_MAX);
            ftrace_info.last_file_name[NAME_MAX - 1] = '\0';  // NULL로 종료
            printk(KERN_INFO "File name: %s\n", ftrace_info.last_file_name);
            path_put(path);
            fput(file);
        }
        free_page((unsigned long)buf);
    }
}

static asmlinkage long ftrace_openat(const struct pt_regs *regs) {
    int fd = regs->si;
    if (current->pid == target_pid) {
        ftrace_info.open_count++;
        get_file_name(fd);
    }
    return real_openat(regs); 
}


static asmlinkage long ftrace_read(const struct pt_regs *regs) {
    ssize_t bytes = regs->dx;  

    if (current->pid == target_pid) {
        ftrace_info.read_count++;  
        ftrace_info.total_read_bytes += bytes;  
    }
    return real_read(regs);  
}


static asmlinkage long ftrace_write(const struct pt_regs *regs) {
    ssize_t bytes = regs->dx; 

    if (current->pid == target_pid) {
        ftrace_info.write_count++; 
        ftrace_info.total_write_bytes += bytes;
    }
    return real_write(regs);  
}


static asmlinkage long ftrace_lseek(const struct pt_regs *regs) {
    if (current->pid == target_pid) {
        ftrace_info.lseek_count++; 
    }
    return real_lseek(regs); 
}


static asmlinkage long ftrace_close(const struct pt_regs *regs) {
    if (current->pid == target_pid) {
        ftrace_info.close_count++;  
    }
    return real_close(regs);  
}


void make_rw(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    if (pte->pte & ~_PAGE_RW)
        pte->pte |= _PAGE_RW;
}


void make_ro(void *addr) {
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    pte->pte = pte->pte & ~_PAGE_RW;
}
//EXPORT_SYMBOL(ftrace_info);

static int __init iotracehooking_init(void) {
    // Find system call table
    syscall_table = (void **)kallsyms_lookup_name("sys_call_table");
    
    // Change permission of the page of system call table(read, write)
    make_rw(syscall_table);
    
    real_openat = (void *)syscall_table[__NR_openat];
    real_read = (void *)syscall_table[__NR_read];
    real_write = (void *)syscall_table[__NR_write];
    real_lseek = (void *)syscall_table[__NR_lseek];
    real_close = (void *)syscall_table[__NR_close];

    syscall_table[__NR_openat] = (void *)ftrace_openat;
    syscall_table[__NR_read] = (void *)ftrace_read;
    syscall_table[__NR_write] = (void *)ftrace_write;
    syscall_table[__NR_lseek] = (void *)ftrace_lseek;
    syscall_table[__NR_close] = (void *)ftrace_close;

    make_ro(syscall_table);

    return 0;
}

/* Called when the module exits. */
static void __exit iotracehooking_exit(void) {
    // System call restoration
    if (syscall_table[__NR_openat] == (void *)ftrace_openat) {
        make_rw(syscall_table);
        syscall_table[__NR_openat] = real_openat;
        syscall_table[__NR_read] = real_read;
        syscall_table[__NR_write] = real_write;
        syscall_table[__NR_lseek] = real_lseek;
        syscall_table[__NR_close] = real_close;
        
        // Recover the page's permission(read-only)
        make_ro(syscall_table);
    }
}

module_init(iotracehooking_init);
module_exit(iotracehooking_exit);
MODULE_LICENSE("GPL");

