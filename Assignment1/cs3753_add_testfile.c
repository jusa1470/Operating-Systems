#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

int main(){
	int tot;
	int ret = syscall(334, 3, 8, &tot);
	if (ret == 0) {
		printf("Total: %d\n", tot);
	} else {
		printf("Error");
	}
}