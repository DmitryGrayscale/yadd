#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (-1 == access(".secret", F_OK) ||
	    -1 == access(".config", F_OK)    )
	{
		perror("Something was wrong");
		exit(EXIT_FAILURE);
	}

	printf("OK!\n");
	return 0;
}
