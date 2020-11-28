#!/bin/bash

STATUS=$(vcgencmd get_throttled | sed -n 's|^throttled=\(.*\)|\1|p')
if [[ ${STATUS} -ne 0 ]]; then
  if [ $((${STATUS} & 0x00001)) -ne 0 ]; then
    printf "Power now under volt "
  elif [ $((${STATUS} & 0x10000)) -ne 0 ]; then
#    printf "Power was under volt "
    :
  fi
  if [ $((${STATUS} & 0x00002)) -ne 0 ]; then
    printf "ARM Frequency now capped "
  elif [ $((${STATUS} & 0x20000)) -ne 0 ]; then
#    printf "ARM Frequency was capped "
    :
  fi
  if [ $((${STATUS} & 0x00004)) -ne 0 ]; then
    printf "CPU now throttled "
  elif [ $((${STATUS} & 0x40000)) -ne 0 ]; then
#    printf "CPU was throttled "
    :
  fi
  if [ $((${STATUS} & 0x00008)) -ne 0 ]; then
    printf "Temperature now soft limit "
  elif [ $((${STATUS} & 0x80000)) -ne 0 ]; then
#    printf "Temperature was soft limit "
    :
  fi
fi
echo ""
