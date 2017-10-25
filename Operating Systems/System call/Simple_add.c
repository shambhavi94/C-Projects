#include	<linux/kernel.h>
#include	<linux/linkage.h>
asmlinkage	long	sys_Simple_add(int number1, int number2, int *result)
{
	*result = number1 + number2;
	printk (KERN_ALERT "The numbers to be added are  %d and %d \n", number1, number2);
	printk(KERN_ALERT "the answer is = %d\n", *result);
	return 0;
}
