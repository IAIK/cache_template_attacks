#!/bin/bash
if [[ "$3" = "" ]]; then
  echo "usage: ./spy.sh <sleep before test> <test duration> <processname>"
  exit 0
fi
ps=`ps axww`
ps=`echo "$ps" | grep -v "spy" | grep "$3" | head -n 1`
if [[ "$ps" = "" ]]; then
  echo process not found
  exit 0
fi
pid=`echo $ps | grep -oE "^ *[0-9]+" | tr -d ' '`
if [[ "$pid" = "" ]]; then
  echo pid not found
  exit 0
fi
truncate -s 0 log.txt
i=$1
while [[ $i -gt 0 ]]; do
  echo "please prepare... starting test in $i seconds..."
  sleep 1
  i=$((i - 1))
done
mem=`sudo cat /proc/$pid/maps | grep "/" | grep -v "(deleted)" | grep -E "(so|locale)" | grep -E "(r-x|rw-|r--)"`
while read -r line; do
  #echo $line
  #echo ./spy $2 $line
  entry=`./spy 0 $line 2>&1`
  echo "$entry ./spy $2 $line"
  ./spy $2 $line >> log.txt
done <<< "$mem"

