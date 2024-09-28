#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>	// kallsyms_lookup_name()
#include <linux/syscalls.h>	// __SYSCALL_DEFINEx()
#include <asm/syscall_wrapper.h>// __SYSCALL_DEFINEx()


void **syscall_table;

void *real_os_ftrace; // Pointer to store the address of the existing os_ftrace system.


__SYSCALL_DEFINEx(1, my_ftrace, pid_t, pid)
{
	printk("os_ftrace() hooked! os_ftrace -> my_ftrace\n");
	return 0;
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
	syscall_table[__NR_os_ftrace] = __x64_sysmy_ftrace;

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
