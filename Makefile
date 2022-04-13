obj-m += driver_test.o

KDIR = /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
        make -C $(KDIR) M=$(PWD) modulesd

clean:
        make -C $(KDIR) M=$(PWD) clean
