#ifndef FTRACEHOOKING_H
#define FTRACEHOOKING_H

#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <linux/limits.h>


struct ftrace_data {
    int open_count;                
    int close_count;               
    int read_count;                 
    int write_count;               
    int lseek_count;                
    size_t total_read_bytes;       
    size_t total_write_bytes;     
    char last_file_name[NAME_MAX];  
};
extern struct ftrace_data ftrace_info;
extern pid_t target_pid;

#endif 

