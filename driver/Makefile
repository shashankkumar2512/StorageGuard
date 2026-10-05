obj-m += storageguard_driver.o

KDIR := $(HOME)/WSL2-Linux-Kernel
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean