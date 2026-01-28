obj-m += tango32.o

all:
	make -C $(KERNEL_SRC) M=$(M) modules

clean:
	make -C $(KERNEL_SRC) M=$(M) clean
