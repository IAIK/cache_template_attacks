#!/bin/bash
if [[ "$3" = "" ]]; then
  echo "usage: ./spy.sh <sleep before test> <test duration> <binary/library>"
  exit 0
fi
filesize=$(stat -c%s "$3")
if [[ "$filesize" -gt "0" ]]; then
  echo "file size is $filesize bytes"
else
  echo "file does not exist..."
  exit 1
fi
i=$1
while [[ $i -gt 0 ]]; do
  echo "please prepare... starting test in $i seconds..."
  sleep 1
  i=$((i - 1))
done
filesize=$(printf "%x" "$filesize")
./spy $2             0-$filesize  --     0        -- -- $3 | ../../exploitation/multi_spy/spy $3

