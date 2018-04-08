#!/bin/bash

set -eu

apt-get update -y && apt-get upgrade -y

apt-get install -y \
  binutils \
  build-essential \
  clang-4.0 \
  htop \
  lldb \
  python3 \
  python3-pip \
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

#
# install Python packages
#
pip3 install --upgrade pip
pip3 install --requirement /tmp/python-requirements.txt
