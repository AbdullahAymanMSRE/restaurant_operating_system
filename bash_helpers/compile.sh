#!/bin/bash

# Create output directory if it does not exist
if [ ! -d "~/.restaurant/c_output" ]; then
  mkdir ~/.restaurant/c_output
fi

gcc -c ~/.restaurant/c_code/shared.c -o ~/.restaurant/c_output/shared.o -pthread
gcc -c ~/.restaurant/c_code/order_process.c -o ~/.restaurant/c_output/order_process.o -pthread
gcc ~/.restaurant/c_code/client.c ~/.restaurant/c_output/shared.o ~/.restaurant/c_output/order_process.o -o ~/.restaurant/c_output/client -pthread
gcc ~/.restaurant/c_code/staff.c ~/.restaurant/c_output/shared.o ~/.restaurant/c_output/order_process.o -o ~/.restaurant/c_output/staff -pthread
gcc ~/.restaurant/c_code/display.c ~/.restaurant/c_output/shared.o ~/.restaurant/c_output/order_process.o -o ~/.restaurant/c_output/display -pthread
gcc ~/.restaurant/c_code/init_memory.c ~/.restaurant/c_output/shared.o -o ~/.restaurant/c_output/init_memory -pthread
