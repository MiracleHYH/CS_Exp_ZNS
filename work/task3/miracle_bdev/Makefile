SPDK_ROOT_DIR := /home/miracle/work/task2/spdk
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = miracle_bdev

C_SRCS := miracle_bdev.c

SPDK_LIB_LIST = $(ALL_MODULES_LIST) event event_bdev

include $(SPDK_ROOT_DIR)/mk/spdk.app.mk

run: all
	@ echo "Finished Compiling, Cleaning intermediate files"
	@ rm -f miracle_bdev.d miracle_bdev.o
	@ echo "Generating bdev-config"
	@ $(SPDK_ROOT_DIR)/scripts/gen_nvme.sh --json-with-subsystems > ./miracle_bdev.json
	@ echo "Generated bdev-config, Runing Program"
	@ sudo ./miracle_bdev -c ./miracle_bdev.json
	@ echo "Comparing Writing Data and Reading Data"
	@ echo "***************** Data Size *****************"
	@ du -h data.*
	@ echo "******************** End ********************"
	@ echo "Comparing Context ... (Note: Using [diff] command, empty output means no different)"
	@ diff data.in data.out