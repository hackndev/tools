#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

#include <linux/interrupt.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/hardware.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/irqs.h>
#include <asm/uaccess.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>

#define PROCFS_NAME	"gpio"
#define DEBUGFS_NAME	"gpio"

static struct proc_dir_entry *proc_intf;
static struct dentry	     *debugfs_intf;

#define PROCFS_MAX_SIZE		20

static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

#define GPIO_TEST(reg, gp) (reg(gp) & GPIO_bit(gp))
static int dfs_show(struct seq_file *s, void *_)
{
	int i,afn;
	seq_printf(s, "GPIO lines status:\n");

	for(i=0;i<89;i++) {
		afn = (GAFR(i) & (0x3 << (((i) & 0xf)*2))) >> (((i) & 0xf)*2);
		seq_printf(s, "%s%d: %s %s %s %s %lx\n",
				i<10?"0":"",
				i,
				GPIO_TEST(GPLR, i)?"*":" ",
				GPIO_TEST(GPDR, i)?"->":"<-",
				GPIO_TEST(GRER, i)?"_/":"  ",
				GPIO_TEST(GFER, i)?"\\_":"  ",
				afn);

	}
	
	return 0;
}

static int dfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, dfs_show, inode->u.generic_ip);
}

static struct file_operations debug_fops = {
	.open		= dfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void gpio_set(int id, int on)
{
	do {
		if (on)
			GPSR(id) = GPIO_bit(id);
		else
			GPCR(id) = GPIO_bit(id);
	} while (0);
}

void set_afn(int gpio, int fn)
{
	int gafr;
	gafr = GAFR(gpio) & ~(0x3 << (((gpio) & 0xf)*2));
	GAFR(gpio) = gafr |  (fn  << (((gpio) & 0xf)*2));
}

void handle_request()
{
	char *p = NULL;
	unsigned long id = simple_strtoul(procfs_buffer+2, &p, 10);
	switch(procfs_buffer[0]) {
		case 'L':
			gpio_set(id, 1);
			printk(KERN_ERR "GPIOed: GPIO %lu set high\n", id);
			break;
		case 'l':
			gpio_set(id, 0);
			printk(KERN_ERR "GPIOed: GPIO %lu set low\n", id);
			break;
		case 'd':
			GPDR(id) &= ~(GPIO_bit(id));
			break;
		case 'D':
			GPDR(id) |= GPIO_bit(id);
			break;
		case '0':
			set_afn(id, 0);
			break;
		case '1':
			set_afn(id, 1);
			break;
		case '2':
			set_afn(id, 2);
			break;
		default:
			printk(KERN_ERR "GPIOed: Unknown request\n");
			break;
	}
}


int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}
	
	handle_request();
	
	return procfs_buffer_size;
}


static int __init gpioed_init(void)
{
	proc_intf = create_proc_entry(PROCFS_NAME, 0644, NULL);
	if (proc_intf == NULL) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}
	
	/*proc_intf->read_proc = procfile_read;*/
	proc_intf->write_proc = procfile_write;
	proc_intf->owner     = THIS_MODULE;
	proc_intf->mode      = S_IFREG | S_IRUGO;
	proc_intf->uid       = 0;
	proc_intf->gid       = 0;
	proc_intf->size      = 37;
	
	debugfs_intf = debugfs_create_file(DEBUGFS_NAME, S_IRUGO, NULL, NULL, &debug_fops);

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	
        return 0;
}

static void __exit gpioed_exit(void)
{
	debugfs_remove(debugfs_intf);
	remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}


/*** Some more stuff ***/
module_init(gpioed_init);
module_exit(gpioed_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir \"Farcaller\" Pouzanov <farcaller@gmail.com>");
MODULE_DESCRIPTION("GPIO editor for PXA26x, second edition");

