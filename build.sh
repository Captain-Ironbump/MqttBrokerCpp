#!/bin/bash

# get all source files from the src directory
src_files=$(find ./src -name "*.cpp")
# print all source src_files
echo "Source files:"
echo "$src_files"

echo "Compiling..."
# have the src_files as arrays and put them inside the g++ command
output=$(g++ -o main $src_files 2>&1)

if [ $? -ne 0 ]; then
  echo "Compilation failed"
  echo "$output"
  echo "$output" | wl-copy
  echo "The error output og g++ has been copied to the clipboard"
  exit 1
fi

echo "Compilation successful"
