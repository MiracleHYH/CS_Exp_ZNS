SPDK_ROOT_DIR := /home/miracle/work/spdk
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = miracle_blob

C_SRCS := miracle_blob.c

SPDK_LIB_LIST = $(ALL_MODULES_LIST) event event_bdev

include $(SPDK_ROOT_DIR)/mk/spdk.app.mk

run: all
	@ rm -f miracle_blob.d miracle_blob.o
	@ sudo ./miracle_blob ./miracle_bdev.json