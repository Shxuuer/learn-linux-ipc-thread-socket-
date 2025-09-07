obj-m := shdmem.o
obj-m += fifo.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all: fifo_epoll_test.o fifo_signal_test.o
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -o fifo_epoll_test fifo_epoll_test.c
	gcc -o fifo_signal_test fifo_signal_test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f fifo_epoll_test fifo_signal_test