#ifndef FTRACEHOOKING_H
#define FTRACEHOOKING_H

#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>

extern void **syscall_table;
extern pid_t traced_pid;

void make_rw(void *addr);
void make_ro(void *addr);

#endif
