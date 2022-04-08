#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_NAME     "/dev/ioctl_driver"

#define READM   _IOR(0, 0, int32_t *)
#define WRITEM  _IOW(0, 1, int32_t *)

int main()
{
        int fd, rc;
        int32_t readb, writeb;

        fd = open(DEVICE_NAME, O_RDWR);
        if (fd < 0)
        {
                printf("open - error :[%d]\n", errno);
                return (-1);
        }

        printf("Input number: ");
        scanf("%d", &writeb);

        rc = ioctl(fd, WRITEM, (int32_t *)&writeb);
        if (rc < 0)
        {
                close(DEVICE_NAME);
                printf("ioctl write - error!\n");
                return (-1);
        }

        rc = ioctl(fd, READM, (int32_t *)&readb);
        if (rc < 0)
        {
                close(DEVICE_NAME);
                printf("ioctl read - error!\n");
                return (-1);
        }
        printf("readb: [%d]\n", readb);

        return (0);
}
