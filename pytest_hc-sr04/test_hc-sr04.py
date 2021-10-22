#!/usr/bin/env python3

import ctypes
import fcntl
import os
import time

import linux

TEST_HCSR04_MAJ_VER = "1"
TEST_HCSR04_MIN_VER = "0"
DEVICE_FILE = "/dev/hc_sr040"

class DATA(ctypes.Structure):
  _fields_ = [
    ("value", ctypes.c_uint),
    ("status", ctypes.c_uint)
  ]

GPIO_HCSR04_IOC_TYPE = "S"
GPIO_HCSR04_EXEC_MEASURE_DISTANCE = linux.IOR(GPIO_HCSR04_IOC_TYPE, 0x01, ctypes.sizeof(DATA))
GPIO_HCSR04_GET_DISTANCE = linux.IOW(GPIO_HCSR04_IOC_TYPE, 0x02, ctypes.sizeof(DATA))

print("ver " + TEST_HCSR04_MAJ_VER + "." + TEST_HCSR04_MIN_VER)

fd = os.open(DEVICE_FILE, os.O_RDWR)

data = DATA()
data.value = 0
data.status = 0
fcntl.ioctl(fd, GPIO_HCSR04_EXEC_MEASURE_DISTANCE, data)

time.sleep(1)

fcntl.ioctl(fd, GPIO_HCSR04_GET_DISTANCE, data)
print("Get distance: " + str(data.value) + "mm")

os.close(fd)
