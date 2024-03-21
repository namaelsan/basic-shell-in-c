#!/bin/bash
echo "Running tests..."
echo
make all
./myshell > 11.txt &
pid=$(pgrep myshell)
if [[ $pid > 0 ]] ; then
  echo  "exit" > /proc/$pid/fd/0
  sleep 0.5
else
  echo "Fail: exit test failed."
  exit 1
fi

pid=$(pgrep myshell)
if ! $pid; then 
  kill -9 $pid  

  echo "Fail: exit test failed."
  exit 1
fi

echo " Pass: exit test passed."
