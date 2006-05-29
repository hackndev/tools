/*** Basic includes ***/
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

/*** GPIO macros ***/
#define GET_PALMT3_GPIO(gpio) \
	(GPLR(GPIO_NR_PALMT3_ ## gpio) & GPIO_bit(GPIO_NR_PALMT3_ ## gpio))

#define SET_PALMT3_GPIO(gpio, setp) \
	do { \
		if (setp) \
			GPSR(GPIO_NR_PALMT3_ ## gpio) = GPIO_bit(GPIO_NR_PALMT3_ ## gpio); \
		else \
			GPCR(GPIO_NR_PALMT3_ ## gpio) = GPIO_bit(GPIO_NR_PALMT3_ ## gpio); \
	} while (0)

#define SET_PALMT3_GPIO_N(gpio, setp) \
	do { \
		if (setp) \
			GPCR(GPIO_NR_PALMT3_ ## gpio) = GPIO_bit(GPIO_NR_PALMT3_ ## gpio); \
		else \
			GPSR(GPIO_NR_PALMT3_ ## gpio) = GPIO_bit(GPIO_NR_PALMT3_ ## gpio); \
	} while (0)

#define GET_GPIO_REG(reg,gpio) (GP##reg(gpio) & GPIO_bit(gpio))
#define GET_GPIO(gpio) GET_GPIO_REG(LR, gpio)

/*** /proc interface ***/
static struct proc_dir_entry *proc_intf;
#define procfs_name			"gpioed"
#define PROCFS_MAX_SIZE		20

static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

int procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;

	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		ret = sprintf(buffer, "HelloWorld!\n");
	}
	return ret;
}

void handle_request(void);

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

/*** IRQ (GPIO) handling ***/
static struct workqueue_struct *my_workqueue;
#define MY_WORK_QUEUE_NAME "GPIOed"

static void handle_gpio(void* irq)
{
	int gpn = (int)irq;
	printk(KERN_ERR "*** GPIO *** %d *** is *** %s ***\n", gpn, GET_GPIO(gpn) ? "high" : "low ");
}

irqreturn_t gpio_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	static int initialised = 0;
	static struct work_struct task;
	
	if (initialised == 0) {
		INIT_WORK(&task, handle_gpio, dev_id);
		initialised = 1;
	} else {
		PREPARE_WORK(&task, handle_gpio, dev_id);
	}

	queue_work(my_workqueue, &task);

	return IRQ_HANDLED;
}

/*** GPIO R/W ***/
static int gpio_get(int id)
{
	return GET_GPIO(id);
}

static void gpio_set(int id, int on)
{
	do {
		if (on)
			GPSR(id) = GPIO_bit(id);
		else
			GPCR(id) = GPIO_bit(id);
	} while (0);
}

static int gpio_watch(int x)
{
	int ret;
	ret = request_irq (IRQ_GPIO(x), gpio_irq, SA_SAMPLE_RANDOM, "test_handler", (void*)x);
	set_irq_type (IRQ_GPIO(x), IRQT_BOTHEDGE);
	if(ret!=0) {
		printk(KERN_ERR "GPIOed: failed to register for GPIO %d\n", x);
		return 1;
	} else {
		printk(KERN_ERR "GPIOed: Registered GPIO %d\n", x);
		return 0;
	}
}

/*** Request handler ***/
void handle_request()
{
	char *p = NULL;
	unsigned long base = 10;
	unsigned long id;
	
	if((procfs_buffer[0] == 'P') || (procfs_buffer[0] == 'V'))
		base = 16;
	id = simple_strtoul(procfs_buffer+2, &p, base);
	switch(procfs_buffer[0]) {
		case 'r':
			printk(KERN_ERR "GPIOed: GPIO %lu is %s\n", id, gpio_get(id)?"high":"low ");
			break;
		case 's':
			gpio_watch(id);
			break;
		case 'h':
			gpio_set(id, 1);
			printk(KERN_ERR "GPIOed: GPIO %lu set high\n", id);
			break;
		case 'l':
			gpio_set(id, 0);
			printk(KERN_ERR "GPIOed: GPIO %lu set low\n", id);
			break;
		case 'd':
			printk(KERN_ERR "GPIOed: GPIO %lu is %s\n", id, GET_GPIO_REG(DR,id)?"output":"input");
			break;

		case 'P':
			printk(KERN_ERR "GPIOed: P-V for 0x%x is 0x%x\n", id, (unsigned int)phys_to_virt(id));
			break;
		case 'V':
			printk(KERN_ERR "GPIOed: V-P for 0x%x is 0x%x\n", id, (unsigned int)virt_to_phys(id));
			break;
		case 'D':
			base = *((unsigned int*)id);
			printk(KERN_ERR "GPIOed: 0x%x = 0x%x\n", id, base);
			break;
		default:
			printk(KERN_ERR "GPIOed: Unknown request\n");
			break;
	}
}

/*** init&exit ***/
static int __init gpioed_init(void)
{
	my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);
	
	proc_intf = create_proc_entry(procfs_name, 0644, NULL);
	if (proc_intf == NULL) {
		remove_proc_entry(procfs_name, &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				procfs_name);
		return -ENOMEM;
	}
	
	proc_intf->read_proc = procfile_read;
	proc_intf->write_proc = procfile_write;
	proc_intf->owner     = THIS_MODULE;
	proc_intf->mode      = S_IFREG | S_IRUGO;
	proc_intf->uid       = 0;
	proc_intf->gid       = 0;
	proc_intf->size      = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_name);
	
        return 0;
}

static void __exit gpioed_exit(void)
{
	destroy_workqueue(my_workqueue);
	remove_proc_entry(procfs_name, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}


/*** Some more stuff ***/
module_init(gpioed_init);
module_exit(gpioed_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir \"Farcaller\" Pouzanov <farcaller@gmail.com>");
MODULE_DESCRIPTION("GPIO editor for PXA26x");

