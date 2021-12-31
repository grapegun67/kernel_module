#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

//ioctl macro
#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)

int main()
{
	int	fd, rc;
	int32_t	writeb, readb;

	printf("[START]\n");

	fd = open("/dev/test_Dev", O_RDWR);
	if (fd < 0)
	{
		perror("open() error!\n");
		return (-1);
	}

	printf("type new number\n");
	scanf("%d", &writeb);

	printf("Write number to kernel\n");
	rc = ioctl(fd, WR_VALUE, (int32_t *)&writeb);
	if (rc < 0)
	{
		close(fd);
		perror("ioctl() write error!\n");
		return (-1);
	}

	printf("Read written number from kernel\n");
	rc = ioctl(fd, RD_VALUE, (int32_t *)&readb);
	if (rc < 0)
	{
		close(fd);
		perror("ioctl() read error!\n");
		return (-1);
	}

	printf("READ NUMBER: [%d]\n", readb);

	printf("[FINISH]\n");
	close(fd);

	return (0);
}

