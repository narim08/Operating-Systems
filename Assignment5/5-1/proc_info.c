#include <linux/syscalls.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/cred.h>
#include <linux/jiffies.h>

struct proc_dir_entry *proc_info_dir;
struct proc_dir_entry *proc_info_file;
static int targetPid = -1;


static const char* getState(struct task_struct *task)
{
	switch (task->state) {
        	case TASK_RUNNING:
            		return "R (running)";
        	case TASK_INTERRUPTIBLE:
            		return "S (sleeping)";
        	case TASK_UNINTERRUPTIBLE:
            		return "D (disk sleep)";
        	case __TASK_STOPPED:
            		return "T (stopped)";
            	case __TASK_TRACED:
            		return "t (tracing stop)";
            	case EXIT_DEAD:
            		return "X (dead)";
        	case EXIT_ZOMBIE:
            		return "Z (zombie)";
        	case TASK_PARKED:
            		return "P (parked)";
            	case TASK_IDLE:
            		return "I (idle)";
        	default:
            		return "etc";
        }
}

static int proc_info_print(struct seq_file *sf, void *v)
{
	struct task_struct *task;
	
	seq_printf(sf, "%-6s %-6s %-6s %-6s %-10s %-10s %-15s %s\n", "Pid", "PPid", "Uid", "Gid", "utime", "stime", "State", "Name");
	
	seq_printf(sf, "----------------------------------------------------------------------------------\n");
	
	
	if(targetPid == -1) { //all process
		for_each_process(task) {
			unsigned long Utime = jiffies_to_msecs(task->utime) / 1000;
            		unsigned long Stime = jiffies_to_msecs(task->stime) / 1000;
            		
			seq_printf(sf, "%-6d %-6d %-6d %-6d %-10lu %-10lu %-15s %s\n",
			task->pid, task->real_parent->pid, __kuid_val(task->cred->uid), __kgid_val(task->cred->gid), Utime, Stime, getState(task), task->comm);
		}
	}
	else { //target process
		task = pid_task(find_vpid(targetPid), PIDTYPE_PID); //Find task by pid
		if (task) {
			unsigned long Utime = jiffies_to_msecs(task->utime) / 1000;
            		unsigned long Stime = jiffies_to_msecs(task->stime) / 1000;
            		
			seq_printf(sf, "%-6d %-6d %-6d %-6d %-10lu %-10lu %-15s %s\n",
			task->pid, task->real_parent->pid, __kuid_val(task->cred->uid), __kgid_val(task->cred->gid), Utime, Stime, getState(task), task->comm);
		}
	}
	
	return 0;
}


static int proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_info_print, NULL);
}


//input target pid
static ssize_t proc_info_write(struct file *f, const char __user *data_user, size_t len, loff_t *off)
{
	char pidBuffer[256]; //save pid
	int pid;
	
	if (len > sizeof(pidBuffer)) {
		return -EINVAL;
	}
	
	if (copy_from_user(pidBuffer, data_user, len)) {
		return -EFAULT;
	}
	
	pidBuffer[len]='\0'; //set end string
	
	if (kstrtoint(pidBuffer, 10, &pid) == 0) { //convert str to int(pid)
		targetPid = pid; //set tartget Pid
	}
	else {
		return -EINVAL;
	}
	
	return len;
}

static const struct file_operations proc_info_ops = {
	.owner = THIS_MODULE,
	.open = proc_info_open,
	.read = seq_read,
	.write = proc_info_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init proc_info_init(void)
{
	proc_info_dir = proc_mkdir("proc_2022202065", NULL);
	proc_info_file = proc_create("processInfo", 0666, proc_info_dir, &proc_info_ops);
	
	return 0;
}

static void __exit proc_info_exit(void)
{
	remove_proc_entry("processInfo", proc_info_dir);
	remove_proc_entry("proc_2022202065", NULL);
}

module_init(proc_info_init);
module_exit(proc_info_exit);
MODULE_LICENSE("GPL");
