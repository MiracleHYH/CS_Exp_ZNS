#! /bin/bash
cd ~/Documents/CS_Exp_ZNS
qemu-system-x86_64 -name cs-exp-zns -m 8G --enable-kvm -cpu host -smp 4 \
-hda ./env/ubuntu.qcow2 \
-net user,hostfwd=tcp:127.0.0.1:7777-:22,hostfwd=tcp:127.0.0.1:2222-:2000 -net nic \
-drive file=./env/znsssd.qcow2,id=mynvme,format=qcow2,if=none \
-device nvme,serial=baz,id=nvme2 \
-device nvme-ns,id=ns2,drive=mynvme,nsid=2,logical_block_size=4096,physical_block_size=4096,zoned=true,zoned.zone_size=131072,zoned.zone_capacity=131072,zoned.max_open=0,zoned.max_active=0,bus=nvme2 \
-fsdev local,id=fsdev0,path=./work/,security_model=none \
-device virtio-9p-pci,id=fs0,fsdev=fsdev0,mount_tag=hostshare
