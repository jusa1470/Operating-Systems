

#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Julia Sanford");

#define BUFFER_SIZE 1024

/* Define device_buffer and other global data structures you will need here */
int opened = 0;
int closed = 0;

void *mem_pointer;

static char device_buffer[BUFFER_SIZE];


ssize_t simple_char_driver_read (struct file *pfile, char __user *user_buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/* length is the length of the userspace buffer*/
	/* offset will be set to current position of the opened file after read*/
	/* copy_to_user function: source is device_buffer and destination is the userspace buffer *buffer */
	int num_chars_left = BUFFER_SIZE - *offset;
	if (num_chars_left == 0) {
		printk(KERN_ALERT "Nothing left to read.\n");
		return 0;
	}

	copy_to_user(user_buffer, device_buffer + *offset, length);
	printk(KERN_ALERT "Read %d bytes from file.\n", length);

	*offset += length;
	return 0;
}



ssize_t simple_char_driver_write (struct file *pfile, const char __user *user_buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/* length is the length of the userspace buffer*/
	/* current position of the opened file*/
	/* copy_from_user function: destination is device_buffer and source is the userspace buffer *buffer */
	int nums_left = BUFFER_SIZE - *offset;
	int need_to_write;

	if (nums_left == 0) {
		printk(KERN_ALERT "No space to write.\n");
		return 0;
	}

	if (length > nums_left) {
		need_to_write = nums_left;
	}
	else {
		need_to_write = length;
	}

	copy_from_user(device_buffer + *offset, user_buffer, need_to_write);
	printk(KERN_ALERT "Wrote %d bytes to file.\n", need_to_write);

	*offset += need_to_write;
	return length;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	opened++;
	printk(KERN_ALERT "File opened %d time(s).", opened);
	return 0;
}

int simple_char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	closed++;
	printk(KERN_ALERT "File opened %d time(s).", closed);
	return 0;
}

loff_t simple_char_driver_seek (struct file *pfile, loff_t offset, int whence)
{
	/* Update open file position according to the values of offset and whence */
	loff_t temp_pos = pfile->f_pos;
	if (whence == 0) {
		pfile->f_pos = offset;
	}
	else if (whence == 1) {
		pfile->f_pos += offset;
	}
	else if (whence == 2) {
		pfile->f_pos = BUFFER_SIZE - offset;
	}
	if (pfile->f_pos < 0 || pfile->f_pos > BUFFER_SIZE) {
		pfile->f_pos = temp_pos;
		return -1;
	}

	return 0;
}

struct file_operations simple_char_driver_file_operations = {

	.owner   = THIS_MODULE,
	/* add the function pointers to point to the corresponding file operations. look at the file fs.h in the linux souce code*/
	.open = simple_char_driver_open,
	.release = simple_char_driver_close,
	.llseek = simple_char_driver_seek,
	.read = simple_char_driver_read,
	.write = simple_char_driver_write,
};

static int simple_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	/* register the device */
	register_chrdev(123, "simple_character_device", &simple_char_driver_file_operations);
	mem_pointer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	return 0;
}

static void simple_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	/* unregister  the device using the register_chrdev() function. */
	unregister_chrdev(123, "simple_character_device");
	kfree(mem_pointer);
}

/* add module_init and module_exit to point to the corresponding init and exit function*/
module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);