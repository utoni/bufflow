#!/bin/bash

if [ `id -u` -ne 0 ]; then
  echo "$0: This program should be run as root"
  echo "$0: Try to get root .."
  su -l root -c "$(realpath $0)"
  exit $?
fi

sysctl -w kernel.randomize_va_space=0 2>/dev/null
sysctl -w kernel.exec-shield=0 2>/dev/null
echo "done."

