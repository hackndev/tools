/* Test driver for Palm T|T3's I2C
 * looks for LED, vibro, fuckin' winbond bridge
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

int tt3i2c_read_value(struct i2c_client *client, u8 reg)
{
	if (reg < 0x10) /* byte-sized register */
		return i2c_smbus_read_byte_data(client,reg);
	else /* word-sized register */
		return i2c_smbus_read_word_data(client,reg);
}

int tt3i2c_write_value(struct i2c_client *client, u8 reg, u16 value)
{
	if (reg == 0x10) /* Impossible to write - driver error! */
		return -1;
		else if (reg < 0x10) /* byte-sized register */
			return i2c_smbus_write_byte_data(client,reg,value);
		else /* word-sized register */
			return i2c_smbus_write_word_data(client,reg,value);
}


/*** change the nubmers below to probe different I2C addresses ***/
static unsigned short normal_i2c[] = {
	71, 72,
I2C_CLIENT_END };

I2C_CLIENT_INSMOD;

int foo_detect_client(struct i2c_adapter *adapter, int address, int kind)
{
	int a,b;
	printk(KERN_ERR "PALM I2C addr = %d  kind = %d\n", address, kind);
	for(a=0;a<0xFFFFFFFF;a++) for(b=0; b<0xFFFFFFFF; b++) ;
	return 0;
}

int tt3i2c_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, &foo_detect_client);
}

static struct i2c_driver tt3i2c_driver = {
	.owner		= THIS_MODULE,
	.name		= "Palm T|T3 I2C driver",
	.flags		= I2C_DF_NOTIFY,
	.attach_adapter	= &tt3i2c_attach_adapter,
//	.detach_client	= &tt3i2c_detach_client,
//	.command	= &tt3i2c_command /* may be NULL */
};

static int __init tt3i2c_init(void)
{
	int res;
	res = i2c_add_driver(&tt3i2c_driver);
	printk("PALM I2C: add driver %d\n", res);
	return 0;
}

static void __exit tt3i2c_exit(void)
{
	i2c_del_driver(&tt3i2c_driver);
}


module_init(tt3i2c_init);
module_exit(tt3i2c_exit);

MODULE_LICENSE("GPL");
