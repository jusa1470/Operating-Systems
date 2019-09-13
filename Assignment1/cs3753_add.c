#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_cs3753_add(int num1, int num2, int *ptr)
{
    int tot;
	printk(KERN_ALERT "First number: %d\n", num1);
    printk(KERN_ALERT "Second number: %d\n", num2);
    tot = num1 + num2;
	put_user(tot, ptr);
    printk(KERN_ALERT "Total: %d\n", tot);
	return 0;
} 
