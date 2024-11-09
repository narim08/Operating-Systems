#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>
#include <asm/ptrace.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/sched/mm.h>
#include <linux/mm_types.h>
#include <linux/kallsyms.h>
#include <linux/init_task.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Process_tracer Module");
MODULE_VERSION("1.0");

void **syscall_table;
void *real_os_ftrace; //Pointer to store the address of the existing os_ftrace system.


/* os_ftrace -> file_varea system call hijack */
asmlinkage pid_t file_varea(const struct pt_regs *regs) {
	struct task_struct *task;
	struct mm_struct *mm;
	struct vm_area_struct *vm;

	pid_t trace_task = (pid_t)regs->di; //get pid
	task = pid_task(find_vpid(trace_task), PIDTYPE_PID); //Find task by pid
	if (!task) { //can't find it
		return -1;
	}
	mm = get_task_mm(task); //get the memory management structure of the process
	
	
	//=================print process information=================//
	printk(KERN_INFO "######## Loaded files of a process '%s(%d)' in VM ########\n", task->comm, task->pid); //process name, pid

	//print address where information is located
	for (vm = mm->mmap; vm; vm = vm->vm_next) { //start:mmap ~ each area
		if (vm->vm_file) { //When the actual file exists
			char buf[256];
			char *path = d_path(&vm->vm_file->f_path, buf, sizeof(buf));
			if (!IS_ERR(path)) {
				printk(KERN_INFO "mem(%lx~%lx) code(%lx~%lx) data(%lx~%lx) heap(%lx~%lx) %s\n", vm->vm_start, vm->vm_end, mm->start_code, mm->end_code, mm->start_data, mm->end_data, mm->start_brk, mm->brk, path);
			}
		}
	}
	printk(KERN_INFO "#########################################################\n");
	
	mmput(mm); //mm structure reference end
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
static int __init file_varea_init(void)
{
	// Find system call table
	syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
	
	// Change permission of the page of system call table(read, write)
	make_rw(syscall_table);
	real_os_ftrace = syscall_table[__NR_os_ftrace];
	syscall_table[__NR_os_ftrace] = file_varea;
	make_ro(syscall_table);

	return 0;
}

/* Called when the module exits. */
static void __exit file_varea_exit(void)
{
	make_rw(syscall_table);
	
	// System call restoration
	syscall_table[__NR_os_ftrace] = real_os_ftrace;

	// Recover the page's permission(read-only)
	make_ro(syscall_table);
}


module_init(file_varea_init);
module_exit(file_varea_exit);

