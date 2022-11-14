SPDK_ROOT_DIR := /home/miracle/work/task2/spdk
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = miracle_bdev

C_SRCS := miracle_bdev.c

SPDK_LIB_LIST = $(ALL_MODULES_LIST) event event_bdev

include $(SPDK_ROOT_DIR)/mk/spdk.app.mk


run: all
	@ rm -f miracle_bdev.d miracle_bdev.o
	@ sudo ./miracle_bdev