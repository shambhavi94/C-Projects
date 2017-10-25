#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main()
{
	int res;
        syscall(1123,1, 10, &res);
	printf("The answer for the userspace is %d\n", res);	
	return 0;

}
