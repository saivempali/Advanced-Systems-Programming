/*
* ASSIGNMENT 4
*
* chardriver.c
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
* 
*/


#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/version.h>
#include<linux/init.h>
#include<linux/slab.h>
#include<linux/errno.h>
#include<linux/err.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/kernel.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/list.h>


//============= MACROS =================================
#define ramdisk_size (size_t)(16*PAGE_SIZE)

#define REGULAR 0
#define REVERSE 1
#define CDRV_IOC_MAGIC 'Z'

#define container_of(ptr, type, member) ({ const typeof( ((type *)0)->member ) *__mptr = (ptr); (type *)( (char *)__mptr - offsetof(type,member) );})

MODULE_LICENSE("GPL");

//============= DEVICE STRUCTURE =================================
struct asp_mycdrv
{
	struct list_head list;
	struct cdev dev;
	char* ramdisk;
	struct semaphore sem;
	int dir;
	int devNo;
}*devices;

//============= GLOBAL VARIABLES =================================
int major_num;
int minor_num = 0;
static int num_of_devices = 3;
static struct class *asp_mycdrv_class = NULL;

module_param(major_num, int, S_IRUGO);
module_param(minor_num, int, S_IRUGO);
module_param(num_of_devices, int, S_IRUGO);

//============= DEVICE OPEN =================================
static int device_open(struct inode *inode, struct file *fp)
{

	struct asp_mycdrv *dev;
	dev = container_of(inode->i_cdev, struct asp_mycdrv, dev);
	fp->private_data = dev;
	printk(KERN_INFO"DEVICE OPENED\n");
	return 0;
}

//============= DEVICE CLOSE =================================
static int device_close(struct inode *inode, struct file *fp)
{
	printk(KERN_INFO "DEVICE CLOSED\n");
	return 0;
}

//============= DEVICE READ =================================
static ssize_t device_read(struct file *fp, char __user *buf, size_t buflen, loff_t *pos)
{
	int limit = 0, bytes_to_read = 0, i = 0;
	struct asp_mycdrv *dev = fp->private_data;
	ssize_t returnValue = -ENOMEM;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
	if(dev->dir == REVERSE)	// REVERSE SETUP
	{
		limit = *pos;
		bytes_to_read = limit > buflen ? buflen : limit;
		printk("This is bytes to read limit in reverse %d bytes to write %d pos %d", limit, bytes_to_read,(int)*pos);

	}
	else if (dev->dir == REGULAR) // REGULAR SETUP
	{
		limit = ramdisk_size - *pos;
		bytes_to_read = limit > buflen ? buflen : limit;
		printk("This is bytes to read limit in reverse %d bytes to write %d pos %d", limit, bytes_to_read,(int)*pos);
	}
	
	if(bytes_to_read == 0) //CHECKING WHETHER DATA IS AVAILABLE
	{
		up(&dev->sem);
		returnValue = -EINVAL;
		printk(KERN_INFO "NO DATA TO READ \n");
		return returnValue;
	}

	if(dev->dir == REVERSE) // READING FROM DEVICE IN REVERSE DIRECTION
	{

		for(i=0; i < bytes_to_read; i++)
		{
			returnValue = copy_to_user(buf+i, dev->ramdisk+*pos, 1);
			if(returnValue)
			{
				up(&dev->sem);
				returnValue = -EFAULT;
				*pos += 1;
				printk(KERN_INFO "ERROR : DEVICE READ UNSUCCESSFUL\n");
				return returnValue;
			}
			else
				*pos -= 1;	
		}	
	}

	else if(dev->dir == REGULAR) // READING FROM DEVICE IN REGULAR DIRECTION
	{ 
		returnValue = copy_to_user(buf, dev->ramdisk + *pos, bytes_to_read);
		if(returnValue)
		{	
			up(&dev->sem);
			returnValue = -EFAULT;
			printk(KERN_INFO "ERROR : DEVICE READ UNSUCCESSFUL\n");
			return returnValue;
		}
		else
			*pos = *pos + bytes_to_read;
		
	}

	printk(KERN_INFO "End device read bytes read = %d, pos = %d \n", bytes_to_read,(int)*pos);
	up(&dev->sem);
	return bytes_to_read;

}

//============= DEVICE WRITE =================================
ssize_t device_write(struct file *fp, const char __user *buf, size_t buflen, loff_t *pos)
{
	int limit = 0, bytes_to_write = 0, i = 0;
	struct asp_mycdrv *dev = fp->private_data;
	ssize_t returnValue = -ENOMEM;

	printk("This is bytes to write limit %d bytes to write %d pos %d", limit, bytes_to_write,(int)*pos);

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;


	if(dev->dir == REVERSE) // REVERSE SETUP
	{
		limit = *pos;
		bytes_to_write = limit > buflen ? buflen : limit;
		printk("This is bytes to write limit in reverse %d bytes to write %d pos %d", limit, bytes_to_write,(int)*pos);

	}
	else if (dev->dir == REGULAR) // REGULAR SETUP
	{
		limit = ramdisk_size - *pos;
		bytes_to_write = limit > buflen ? buflen : limit;
		printk("This is bytes to write limit in regular %d bytes to write %d pos %d", limit, bytes_to_write,(int)*pos);
	}

	if(bytes_to_write == 0) // CHECKING WHETHER DATA IS AVAILABLE
	{
		up(&dev->sem);
		returnValue = -EINVAL;
		printk(KERN_INFO "ERROR : DEVICE WRITE UNSUCCESSFUL \n");
		return returnValue;
	}

	if(dev->dir == REVERSE) // WRITING TO DEVICE IN REVERSE DIRECTION
	{
		
		for(i=0; i < bytes_to_write; i++)
		{
			returnValue = copy_from_user(dev->ramdisk + *pos, buf+i,1);
			if(returnValue)
			{
				returnValue = -EFAULT;
				up(&dev->sem);
				printk(KERN_INFO"ERROR : DEVICE WRITE UNSUCCESSFUL\n");
				*pos += 1;
				return returnValue;	
			}
			else
				*pos -= 1;
		}
	}
	else if(dev->dir == REGULAR) // WRITING TO DEVICE IN REVERSE DIRECTION
	{
		returnValue = copy_from_user(dev->ramdisk + *pos, buf, bytes_to_write);
		if(returnValue)
		{
			returnValue = -EFAULT;
			up(&dev->sem);
			printk(KERN_INFO"ERROR : DEVICE WRITE UNSUCCESSFUL\n");
			return returnValue;	
		}
			*pos += bytes_to_write;
	}
	
	printk(KERN_INFO "End Device Write, bytes write = %d, pos = %d \n", bytes_to_write,(int)*pos);
	up(&dev->sem);
	return bytes_to_write;
}


//================ DEVICE LSEEK ========================================
static loff_t device_lseek(struct file* file, loff_t offset, int origin)
{
	struct asp_mycdrv *dev = file->private_data;
	loff_t pos;

	printk(KERN_INFO "Start device_lseek\n");
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	printk(KERN_INFO "Origin %d\n", origin);
	switch(origin)
	{
		case 0: // SEEK_SET
			pos = offset;
			pr_info("Seek set position = %ld\n",(long)pos);
			break;

		case 1: //SEEK_CUR
			pos = file->f_pos + offset;
			pr_info("Seek Cur position = %ld\n",(long)pos);
			break;
	
		case 2: //SEEK_END
			pos = ramdisk_size + offset;
			pr_info("Seek end position = %ld\n",(long)pos);
			break;

		default:
			up(&dev->sem);
			printk(KERN_INFO"Error: Origin not valid\n");
			return -EINVAL;
	}

	// out of bound - positive check
	pos = pos < ramdisk_size ? pos : ramdisk_size;

	// out of bound - negative check
	pos = pos >= 0 ? pos : 0;

	file->f_pos = pos;
	pr_info("Seeking to position = %ld\n",(long)pos);
	up(&dev->sem);
	return pos;
}

//======================= DEVICE IOCTL ========================================
static long device_ioctl(struct file *fp, unsigned int c, unsigned long dir)
{
	struct asp_mycdrv *dev = fp->private_data;
	int returnValue = 0;

	printk(KERN_INFO "start device ioctl\n");

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	//printk(KERN_INFO " ioct dir value %lu \n", dir);

	if(dir == REGULAR) // REGULAR 
	{
		dev->dir = REGULAR;
	}

	else if(dir == REVERSE)	// REVERSE
	{
		dev->dir = REVERSE;
	}

	else
	{
		up(&dev->sem);
		printk(KERN_INFO"ERROR : ENTERED INVALID DIRECTION\n");
		return -ENOTTY;
	}

	printk(KERN_INFO "End device ioctl\n");
	up(&dev->sem);
	return returnValue;
}

//================= FILE OPERATIONS STRUCTURE ====================
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read,
	.llseek = device_lseek,
	.unlocked_ioctl = device_ioctl
};

//====================== CDEV SETUP ===============================
static int setup_cdev(struct asp_mycdrv *cdrv, struct class *asp_mycdrv_class, int minor_num)
{	
	int result = 0;
	dev_t devNo = MKDEV(major_num, minor_num);
	struct device *devicenode = NULL;
	printk(KERN_INFO "Start setup_dev \n");
	
	cdev_init(&cdrv->dev, &fops);
	cdrv->dev.owner = THIS_MODULE;
	cdrv->dev.ops = &fops;
	result = cdev_add(&cdrv->dev, devNo, 1);
	if( result < 0)
	{
		if(cdrv->ramdisk)
			kfree(cdrv->ramdisk);
		cdev_del(&cdrv->dev);
		return -1;
	}

	devicenode = device_create(asp_mycdrv_class, NULL, devNo, NULL, "mycdrv%d",minor_num);

	if(IS_ERR(devicenode))
	{
		if(cdrv->ramdisk)
			kfree(cdrv->ramdisk);
		cdev_del(&cdrv->dev);
		result = PTR_ERR(devicenode);
		printk(KERN_INFO "ERROR : DEVICE CREATION UNSUCCESSFUL %d %s%d", result, "mycdrv", minor_num);
		return result;
	}

	printk(KERN_INFO "end setup_cdev \n");
	return 0;
}



//===================== DEVICE CLEANUP ================================
void device_cleanup_module(void)
{
	int i=0;
	printk(KERN_INFO "Start Cleanup module\n");				
	if(devices)
	{
		for(i=0; i < num_of_devices; i++)
		{
			//printk(KERN_INFO "Clean device node %d\n",i);
			device_destroy(asp_mycdrv_class,MKDEV(major_num,i));
			if(devices[i].ramdisk)
				kfree(devices[i].ramdisk);
			cdev_del(&devices[i].dev);
		}
		
		if(devices) // FREEING ALL DEVICES
		{
			//printk(KERN_INFO"Freeing All devices %d\n",i);
			kfree(devices);
		}
	}

	if(asp_mycdrv_class) // DESTROYING THE DEVICE CLASS
		class_destroy(asp_mycdrv_class);

	// DEALLOCATING THE MAJOR & MINOR NUMBERS => DEALLOCATING MEMORY REGIONS
	unregister_chrdev_region(MKDEV(major_num,0),num_of_devices);
	printk(KERN_INFO"End Cleanup module\n");
	return;
}

//==================== DEVICE INIT =====================================
int device_init_module(void)
{
	int result=0, i,numDevices=0;
	dev_t dev = 0;


	if(alloc_chrdev_region(&dev, minor_num, num_of_devices, "mycdrv") < 0)	
	{
		printk(KERN_WARNING"Cannot get major number:%d\n",major_num);
		return result;
	}
	major_num = MAJOR(dev);

	/** 
	* Creating the requested number of devices.
	* The devices are handled by the same major number as they are of the same type(char devices).
	* Each device is different from the other in its minor number.
	*/


	devices = kmalloc(num_of_devices*sizeof(struct asp_mycdrv), GFP_KERNEL);

	if(!devices)	/* checking whether the devices were created or not */
	{
		result = -ENOMEM;
		unregister_chrdev_region(MKDEV(major_num,0), num_of_devices);
		return result;
	}

	asp_mycdrv_class = class_create(THIS_MODULE,"mycdrv");
	if(IS_ERR(asp_mycdrv_class))
		return -1;

	memset(devices, 0, num_of_devices*sizeof(struct asp_mycdrv));
	
	for(i=0; i<num_of_devices; i++)
	{
		devices[i].ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
		
		if(!devices[i].ramdisk)
		{
			numDevices = i;
			printk(KERN_INFO "ERROR: Could not allocate memory for device %d\n",i);
			result = -ENOMEM;
			num_of_devices = i;
			return result;
		}

		memset(devices[i].ramdisk,0,ramdisk_size);

		if(i == 0)
		{
			devices[i].list.next = &devices[0].list;
			devices[i].list.prev = &devices[0].list;
		}
		else
		{
			list_add(&devices[i].list, &devices[0].list);
		}
		devices[i].devNo = i;
		sema_init(&devices[i].sem,1);

		result = setup_cdev(&devices[i], asp_mycdrv_class, i);
		if(result)
		{
			if(devices[i].ramdisk)
				kfree(devices[i].ramdisk);
		}
	}
	
	printk(KERN_INFO "End Init Module\n");

	return 0;
}

//==================== DEVICE EXIT ===========================
static void device_exit_module(void)
{
	printk(KERN_INFO "Start Exit Module");
	device_cleanup_module();
	printk(KERN_INFO "End Exit Module");
	return;
}

module_init(device_init_module);
module_exit(device_exit_module);
