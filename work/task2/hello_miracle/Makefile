SPDK_ROOT_DIR := $(abspath $(CURDIR)/../spdk)

APP = hello_miracle

include $(SPDK_ROOT_DIR)/mk/nvme.libtest.mk

run: hello_miracle
	rm -f hello_miracle.d hello_miracle.o
	sudo ./hello_miracle