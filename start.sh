#! /bin/bash

if [ $1 == ssd ]; then
    qemu-system-x86_64 --enable-kvm \
    -nographic \
    -name cs-exp-zns \
    -m 8G \
    -cpu host -smp 4 \
    -hda ./env/ubuntu.qcow2 \
    -net user,hostfwd=tcp:127.0.0.1:7777-:22 -net nic \
    -drive file=./env/ssd.qcow2,id=nvm,if=none \
    -device nvme,serial=baz,drive=nvm \
    -fsdev local,id=fsdev0,path=./work/,security_model=none \
    -device virtio-9p-pci,id=fs0,fsdev=fsdev0,mount_tag=hostshare
else
    qemu-system-x86_64 --enable-kvm \
    -nographic \
    -name cs-exp-zns \
    -m 8G \
    -cpu host -smp 4 \
    -hda ./env/ubuntu.qcow2 \
    -net user,hostfwd=tcp:127.0.0.1:7777-:22 -net nic \
    -drive file=./env/znsssd.qcow2,id=mynvme,if=none \
    -device nvme,serial=baz,id=nvme2 \
    -device nvme-ns,id=ns2,drive=mynvme,nsid=2,logical_block_size=4096,physical_block_size=4096,zoned=true,zoned.zone_size=131072,zoned.zone_capacity=131072,zoned.max_open=0,zoned.max_active=0,bus=nvme2 \
    -fsdev local,id=fsdev0,path=./work/,security_model=none \
    -device virtio-9p-pci,id=fs0,fsdev=fsdev0,mount_tag=hostshare
fi