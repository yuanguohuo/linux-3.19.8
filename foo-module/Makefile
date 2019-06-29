CONFIG_MODULE_SIG=n

obj-m := foo.o

KERN_PATH = ../

all:
	make -C $(KERN_PATH) M=$(PWD) modules


clean:
	make -C $(KERN_PATH) M=$(PWD) clean
