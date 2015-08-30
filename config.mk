#LINUX_PATH:=/lib/modules/$(shell uname -r)/build
LINUX_PATH:=/home/amir/raspi/linux
CROSS_PATH:=/home/amir/raspi/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
FLAGS=\
	ARCH=arm

CMD=make -C $(LINUX_PATH) $(FLAGS) CROSS_COMPILE=$(CROSS_PATH) M=$(PWD)
