#!/bin/bash

if [ `id -u` -ne 0 ]; then
  echo "$0: This script should be run as root."
  echo "$0: Try to get root .."
  sudo "$(readlink -f $0)"
  exit $?
fi

cat /proc/cpuinfo | grep -oq pae 2>/dev/null >/dev/null
ret=$?
if [ $ret -eq 0 ]; then
  echo "$0: PAE enabled system found."
  echo "$0: Some exploits may not work!"
fi

set -x
sysctl -w kernel.randomize_va_space=0 2>/dev/null
sysctl -w kernel.exec-shield=0 2>/dev/null

