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
