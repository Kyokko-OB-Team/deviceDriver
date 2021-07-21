# kernel tree path
KERNEL_PATH	:= /mnt/4tb/raspberrypi_kernel/linux

# ビルドファイルの設定
SrcName = $(lastword $(subst /, ,$(PWD)))
obj-m := $(SrcName).o

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
