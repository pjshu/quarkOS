#!/bin/bash

qemu-system-i386                                 \
  -m 128                                         \
  -drive format=raw,media=disk,file=cmake-build-debug/userspace/disk.img,if=ide,index=0   \
  -drive format=raw,media=cdrom,file=cmake-build-debug/kernel/quarkOS.iso \
  -smp 4 \
  -boot d \
  -nic user,model=e1000
