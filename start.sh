#! /bin/bash

ZNSSSD_NAME=znsssd.qcow2
ZNSSSD_FORMAT=qcow2

if [ $# == 0 ]; then
   echo "未指定ZNS SSD磁盘格式,使用默认qcow2格式文件"
elif [ $1 == qcow2 ]; then
    echo "已指定ZNS SSD磁盘格式为qcow2,将使用znsssd.qcow2,format=qcow2"
elif [ $1 == raw ]; then
    echo "已指定ZNS SSD磁盘格式为raw,将使用znsssd.img,format=raw"
    $ZNSSSD_NAME=znsssd.img
    $ZNSSSD_FORMAT=raw
else
    echo "未知格式(可选:qcow2,raw)"
    exit
fi

cd /home/miracle/Documents/CS_Exp_ZNS

if [ -e ./env/$ZNSSSD_NAME ]; then
    echo "已找到$ZNSSSD_NAME,开始启动虚拟机"
else
    echo "未找到$ZNSSSD_NAME,请先在env中创建ZNS SSD磁盘文件"
    exit
fi

qemu-system-x86_64 --enable-kvm \
-nographic \
-name cs-exp-zns \
-m 8G \
-cpu host -smp 4 \
-hda ./env/ubuntu.qcow2 \
-net user,hostfwd=tcp:127.0.0.1:7777-:22,hostfwd=tcp:127.0.0.1:2222-:2000 -net nic \
-drive file=./env/$ZNSSSD_NAME,id=mynvme,format=$ZNSSSD_FORMAT,if=none \
-device nvme,serial=baz,id=nvme2 \
-device nvme-ns,id=ns2,drive=mynvme,nsid=2,logical_block_size=4096,physical_block_size=4096,zoned=true,zoned.zone_size=131072,zoned.zone_capacity=131072,zoned.max_open=0,zoned.max_active=0,bus=nvme2 \
-fsdev local,id=fsdev0,path=./work/,security_model=none \
-device virtio-9p-pci,id=fs0,fsdev=fsdev0,mount_tag=hostshare
