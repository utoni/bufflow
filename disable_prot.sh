#!/bin/sh

if [ `id -u` -ne 0 ]; then
  echo "$0: This program should be run as root"
fi

sysctl -w kernel.randomize_va_space=0 2>/dev/null
sysctl -w kernel.exec-shield=0 2>/dev/null

