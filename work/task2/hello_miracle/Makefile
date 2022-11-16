SPDK_ROOT_DIR := /home/miracle/work/spdk

APP = hello_miracle

include $(SPDK_ROOT_DIR)/mk/nvme.libtest.mk

run: all
	@ rm -f hello_miracle.d hello_miracle.o
	@ sudo ./hello_miracle