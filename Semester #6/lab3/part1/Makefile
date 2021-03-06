obj-m += md.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

go:
	sudo dmesg -C
	sudo insmod md.ko
	sudo rmmod md.ko
	sudo dmesg