obj-m += driver_test.o

KDIR = /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
        make -C $(KDIR) M=$(PWD) modules
        rm -rf *.o *.mod* modules.order Module.symvers

clean:
        make -C $(KDIR) M=$(PWD) clean
