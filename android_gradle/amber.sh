#!/usr/bin/env bash

args=()
index=1

# The @Q expansion operator requires Bash 4.4.

for arg in "$@"
do
args+=("-e")
args+=("arg${index}")
args+=("${arg}")
index=$((index+1))
done

adb shell rm -f /sdcard/amber_stdout.txt
adb shell rm -f /sdcard/amber_stderr.txt
adb shell am instrument -w \
  -e stdout /sdcard/amber_stdout.txt \
  -e stderr /sdcard/amber_stderr.txt \
  "${args[@]@Q}" \
  com.google.amber.test/androidx.test.runner.AndroidJUnitRunner
adb shell cat /sdcard/amber_stdout.txt
adb shell cat /sdcard/amber_stderr.txt
