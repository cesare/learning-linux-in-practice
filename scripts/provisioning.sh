#!/bin/bash

set -eu

apt-get update -y && apt-get upgrade -y

apt-get install -y \
  binutils \
  build-essential \
  clang-4.0 \
  silversearcher-ag \
  sysstat

#
# enable sysstat
#
echo 'ENABLED="true"' > /etc/default/sysstat
service sysstat restart

#
# disable hyper-threading
#
echo 0 > /sys/devices/system/cpu/cpu1/online
echo 0 > /sys/devices/system/cpu/cpu3/online
